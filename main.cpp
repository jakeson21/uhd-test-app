#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <memory>
#include <csignal>
#include <atomic>
#include <chrono>

#include <DataSource.h>
#include <Radio.h>

namespace po = boost::program_options;
std::atomic<bool> running{false};

void signalHandler(int signum) {
   std::cout << "Interrupt signal (" << signum << ") received.\n";

   // cleanup and close up stuff here  
   // terminate program  

   running = false;
}


int UHD_SAFE_MAIN(int argc, char* argv[])
{
    // register signal SIGINT and signal handler  
    signal(SIGINT, signalHandler);  

    // variables to be set by po
    std::string args;
    std::string wire;
    double seconds_in_future;
    size_t total_num_samps;
    double rate;
    double gain = 30;
    double freq = 251.0e6;
    float ampl;

    // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single uhd device address args")
        // ("wire", po::value<std::string>(&wire)->default_value(""), "the over the wire type, sc16, sc8, etc")
        // ("secs", po::value<double>(&seconds_in_future)->default_value(1.5), "number of seconds in the future to transmit")
        ("rate", po::value<double>(&rate)->default_value(100e3), "rate of outgoing samples")
        ("freq", po::value<double>(&freq)->default_value(250.0e6), "center frequency Hz")
        ("gain", po::value<double>(&gain)->default_value(30), "gain dB")
        // ("ampl", po::value<float>(&ampl)->default_value(float(0.3)), "amplitude of each sample")
        // ("dilv", "specify to disable inner-loop verbose")
    ;
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print the help message
    if (vm.count("help")) {
        std::cout << boost::format("UHD TX Timed Samples %s") % desc << std::endl;
        return ~0;
    }

    auto data_p = std::make_shared<DataSource>();
    Radio myRadio(data_p);
    
    myRadio.configure(rate, freq, gain, args);
    myRadio.start();

    running = true;

    std::cout << "Started..." << std::endl;
    std::cout << "Running until ctrl+c is pressed" << std::endl;
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    myRadio.stop();

    // finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
};