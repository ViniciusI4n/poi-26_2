#include "Metrics.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "StudentRepository.h"

namespace
{
    double nowSeconds()
    {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration<double>(now).count();
    }

    void writeHeader(std::ostringstream& out, const std::string& name,
                     const std::string& help, const std::string& type)
    {
        out << "# HELP " << name << " " << help << "\n";
        out << "# TYPE " << name << " " << type << "\n";
    }

#ifdef __linux__
    void writeProcessMetrics(std::ostringstream& out)
    {
        long pageSize = sysconf(_SC_PAGESIZE);
        std::ifstream statm("/proc/self/statm");
        unsigned long virtualPages = 0;
        unsigned long residentPages = 0;

        if (statm >> virtualPages >> residentPages)
        {
            writeHeader(out, "process_resident_memory_bytes", "Resident memory size in bytes.",
                        "gauge");
            out << "process_resident_memory_bytes " << residentPages * pageSize << "\n";
            writeHeader(out, "process_virtual_memory_bytes", "Virtual memory size in bytes.",
                        "gauge");
            out << "process_virtual_memory_bytes " << virtualPages * pageSize << "\n";
        }

        rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) == 0)
        {
            double cpu = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6 +
                         usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
            writeHeader(out, "process_cpu_seconds_total",
                        "Total user and system CPU time spent in seconds.", "counter");
            out << "process_cpu_seconds_total " << cpu << "\n";
        }
    }
#endif
}

Metrics::Metrics(const StudentRepository& repository)
    : repository_(repository),
      startTime_(nowSeconds())
{
}

void Metrics::recordRequest(const std::string& method, const std::string& route, int status,
                            double seconds)
{
    std::lock_guard<std::mutex> lock(mutex_);
    Histogram& h = requests_[Key(method, route, status)];

    for (std::size_t i = 0; i < kBuckets.size(); i++)
    {
        if (seconds <= kBuckets[i])
        {
            h.buckets[i]++;
        }
    }
    h.count++;
    h.sum += seconds;
}

std::string Metrics::render() const
{
    std::ostringstream out;
    out.precision(9);

    writeHeader(out, "http_server_requests_seconds", "Duration of HTTP requests in seconds.",
                "histogram");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [key, h] : requests_)
        {
            const auto& [method, route, status] = key;
            std::string labels = "method=\"" + method + "\",route=\"" + route +
                                 "\",status=\"" + std::to_string(status) + "\"";

            for (std::size_t i = 0; i < kBuckets.size(); i++)
            {
                out << "http_server_requests_seconds_bucket{" << labels << ",le=\""
                    << kBuckets[i] << "\"} " << h.buckets[i] << "\n";
            }
            out << "http_server_requests_seconds_bucket{" << labels << ",le=\"+Inf\"} "
                << h.count << "\n";
            out << "http_server_requests_seconds_sum{" << labels << "} " << h.sum << "\n";
            out << "http_server_requests_seconds_count{" << labels << "} " << h.count << "\n";
        }
    }

    writeHeader(out, "students_in_memory", "Number of students held in memory.", "gauge");
    out << "students_in_memory " << repository_.size() << "\n";

    writeHeader(out, "students_history_bytes",
                "Bytes held in memory by the students' grade history.", "gauge");
    out << "students_history_bytes " << repository_.historyBytes() << "\n";

    writeHeader(out, "process_start_time_seconds",
                "Start time of the process since unix epoch in seconds.", "gauge");
    out << "process_start_time_seconds " << std::fixed << std::setprecision(3) << startTime_
        << std::defaultfloat << std::setprecision(9) << "\n";

#ifdef __linux__
    writeProcessMetrics(out);
#endif

    return out.str();
}
