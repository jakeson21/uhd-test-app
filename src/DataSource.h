#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <vector>
#include <complex>
#include <random>

class DataSource {
public:
    DataSource()
    {
        gen.seed(rd());
    }

    virtual ~DataSource(){}

    bool getNextData(std::vector<std::complex<float>>& outData, size_t count);

protected:
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<float> d{0,1};
};

#endif