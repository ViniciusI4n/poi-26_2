#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

class StudentRepository;

class Metrics
{
public:
    explicit Metrics(const StudentRepository& repository);

    void recordRequest(const std::string& method, const std::string& route, int status,
                       double seconds);

    std::string render() const;

private:
    static constexpr std::array<double, 12> kBuckets = {
        0.001, 0.0025, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0};

    struct Histogram
    {
        std::array<std::uint64_t, kBuckets.size()> buckets{};
        std::uint64_t count = 0;
        double sum = 0.0;
    };

    using Key = std::tuple<std::string, std::string, int>;

    const StudentRepository& repository_;
    double startTime_;
    std::map<Key, Histogram> requests_;
    mutable std::mutex mutex_;
};
