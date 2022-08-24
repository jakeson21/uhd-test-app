#include <Radio.h>
#include <boost/format.hpp>

Radio::Radio(std::shared_ptr<DataSource> inDataSource)
: data_ptr(inDataSource)
{

}

Radio::~Radio()
{
    running = false;
    if (run_thread.joinable())
    {
        run_thread.join();
    }
}

bool Radio::configure(double sampRate, double centerFreq, double inGain, const std::string& inArgs)
{
    this->samp_rate = sampRate;
    this->center_freq = centerFreq;
    this->gain = inGain;
    this->args = inArgs;

    if (running)
    {
        stop();
        if (this->run_thread.joinable())
        {
            this->run_thread.join();
        }
    }

    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args
              << std::endl;
    usrp = uhd::usrp::multi_usrp::make(args);
    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

     // set the tx sample rate
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (samp_rate / 1e6) << std::endl;
    usrp->set_tx_rate(samp_rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate() / 1e6)
              << std::endl
              << std::endl;

    
    usrp->set_tx_gain(gain);
            std::cout << boost::format("Actual TX Gain: %f dB...") % usrp->get_tx_gain()
            << std::endl
            << std::endl;

    double lo_offset = 0;
    uhd::tune_request_t tune_request(center_freq, lo_offset);
    usrp->set_tx_freq(tune_request);
    std::cout << boost::format("Actual TX Freq: %f Mhz...") % (usrp->get_tx_freq() / 1e6)
                << std::endl
                << std::endl;

    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
    usrp->set_time_now(uhd::time_spec_t(0.0));

    // create a transmit streamer
    uhd::stream_args_t stream_args("fc32", ""); // complex floats
    tx_stream = usrp->get_tx_stream(stream_args);

    // allocate buffer with data to send
    tx_buff = std::vector<std::complex<float>>(tx_stream->get_max_num_samps(), std::complex<float>(1, 0));

    return true;
}
    
bool Radio::start()
{
    run_thread = std::thread(&Radio::run, this);
    return true;
}

void Radio::stop()
{
    running = false;
}

void Radio::run()
{
     // setup metadata for the first packet
     double seconds_in_future = 1.0;
    uhd::tx_metadata_t md;
    md.start_of_burst = false;
    md.end_of_burst   = false;
    md.has_time_spec  = true;
    md.time_spec      = uhd::time_spec_t(seconds_in_future);

    // the first call to send() will block this many seconds before sending:
    const double timeout =
        seconds_in_future + 0.1; // timeout (delay before transmit + padding)

    size_t num_acc_samps = 0; // number of accumulated samples

    running = true;
    while(running)
    {
        size_t samps_to_send = tx_buff.size();

        data_ptr->getNextData(tx_buff, samps_to_send);

        // send a single packet
        size_t num_tx_samps = tx_stream->send(&tx_buff.front(), samps_to_send, md, timeout);

        // do not use time spec for subsequent packets
        md.has_time_spec = false;

        if (num_tx_samps < samps_to_send) std::cerr << "Send timeout..." << std::endl;

        num_acc_samps += num_tx_samps;

        // if (verbose)
        //     std::cout << boost::format("Sent packet: %u samples") % num_tx_samps
        //               << std::endl;
    }

    // send a mini EOB packet
    md.end_of_burst = true;
    tx_stream->send("", 0, md);

    std::cout << std::endl << "Waiting for async burst ACK... " << std::flush;
    uhd::async_metadata_t async_md;
    bool got_async_burst_ack = false;
    // loop through all messages for the ACK packet (may have underflow messages in queue)
    while (not got_async_burst_ack and tx_stream->recv_async_msg(async_md, timeout)) {
        got_async_burst_ack =
            (async_md.event_code == uhd::async_metadata_t::EVENT_CODE_BURST_ACK);
    }
    std::cout << (got_async_burst_ack ? "success" : "fail") << std::endl;
}
