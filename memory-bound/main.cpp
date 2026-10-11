#include <chrono>
#include <iostream>
#include <string>

#include "CsvLoader.h"
#include "StudentController.h"
#include "StudentRepository.h"
#include "httplib.h"

int main(int argc, char* argv[])
{
    int port = 8080;
    std::string csvPath = "data/students.csv";
    std::size_t scale = 0;

    for (int i = 1; i + 1 < argc; i += 2)
    {
        std::string flag = argv[i];
        std::string value = argv[i + 1];

        if (flag == "--port")
        {
            port = std::stoi(value);
        }
        else if (flag == "--csv")
        {
            csvPath = value;
        }
        else if (flag == "--scale")
        {
            scale = std::stoull(value);
        }
        else
        {
            std::cerr << "unknown option: " << flag << "\n";
            return 1;
        }
    }

    try
    {
        auto start = std::chrono::steady_clock::now();

        std::vector<Student> students = csv::load(csvPath);
        std::cout << "loaded " << students.size() << " students from " << csvPath << "\n";

        if (scale > 0)
        {
            students = csv::scaleUp(students, scale);
            std::cout << "scaled to " << students.size() << " students\n";
        }

        StudentRepository repository;
        repository.load(std::move(students));

        if (scale == 0)
        {
            repository.enablePersistence(csvPath);
        }
        else
        {
            std::cout << "scaled mode: changes stay in memory, CSV is not touched\n";
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::cout << "data ready in " << ms << " ms\n";

        StudentController controller(repository);
        httplib::Server server;
        controller.registerRoutes(server);

        std::cout << "listening on port " << port << "\n";
        if (!server.listen("0.0.0.0", port))
        {
            std::cerr << "could not bind port " << port << "\n";
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
