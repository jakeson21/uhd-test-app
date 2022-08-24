#include <DataSource.h>

bool DataSource::getNextData(std::vector<std::complex<float>>& outData, size_t count)
{
    for (auto& x : outData)
    {
        x = float(0.7) + float(0.3)*std::complex<float>{d(gen), d(gen)};
    }

    return true;
}