#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>

#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <complex>

#include <DataSource.h>

#ifndef RADIO_H
#define RADIO_H

class Radio {
public:
    Radio(std::shared_ptr<DataSource> inDataSource);
    virtual ~Radio();

    bool configure(double sampRate, double centerFreq, double gain, const std::string& args);
    
    bool start();
    void stop();
protected:
    void run(); 
    std::mutex stop_mutex;
    std::atomic<bool> running;
    std::thread run_thread;

    double samp_rate;
    double center_freq;
    double gain;
    std::string args;

    uhd::usrp::multi_usrp::sptr usrp;
    uhd::tx_streamer::sptr tx_stream;
    std::vector<std::complex<float>> tx_buff;

    // Source of data for TX
    std::shared_ptr<DataSource> data_ptr;
};

#endif