#include "CsvLoader.h"

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace csv
{
    namespace
    {
        std::vector<std::string> splitLine(const std::string& line)
        {
            std::vector<std::string> fields;
            std::stringstream stream(line);
            std::string field;

            while (std::getline(stream, field, ','))
            {
                fields.push_back(field);
            }

            return fields;
        }
    }

    std::vector<Student> load(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("cannot open file: " + path);
        }

        std::vector<Student> students;
        std::string line;
        std::getline(file, line);

        while (std::getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            if (line.back() == '\r')
            {
                line.pop_back();
            }

            std::vector<std::string> f = splitLine(line);
            if (f.size() != 8)
            {
                throw std::runtime_error("bad line: " + line);
            }

            students.emplace_back(std::stoi(f[0]), f[1], f[2], std::stoi(f[3]),
                                  std::stod(f[4]), std::stod(f[5]),
                                  std::stod(f[6]), std::stod(f[7]));
        }

        return students;
    }

    void save(const std::string& path, const std::vector<Student>& students)
    {
        std::string tempPath = path + ".tmp";

        {
            std::ofstream file(tempPath);
            if (!file.is_open())
            {
                throw std::runtime_error("cannot write file: " + tempPath);
            }

            file << "id,name,course,age,grade1,grade2,grade3,attendance\n";
            for (const Student& s : students)
            {
                file << s.getId() << ',' << s.getName() << ',' << s.getCourse() << ','
                     << s.getAge() << ',' << s.getGrade1() << ',' << s.getGrade2() << ','
                     << s.getGrade3() << ',' << s.getAttendance() << '\n';
            }

            if (!file.good())
            {
                throw std::runtime_error("failed while writing: " + tempPath);
            }
        }

        std::filesystem::rename(tempPath, path);
    }

    std::vector<Student> scaleUp(const std::vector<Student>& seed, std::size_t target)
    {
        if (seed.empty())
        {
            throw std::runtime_error("seed is empty");
        }

        std::vector<Student> result;
        result.reserve(target);

        std::mt19937 rng(42);
        std::uniform_real_distribution<double> grade(0.0, 10.0);
        std::uniform_real_distribution<double> attendance(50.0, 100.0);
        std::uniform_int_distribution<int> age(17, 30);

        for (std::size_t i = 0; i < target; i++)
        {
            const Student& base = seed[i % seed.size()];
            int id = static_cast<int>(i) + 1;
            std::string name = base.getName() + " #" + std::to_string(id);

            result.emplace_back(id, name, base.getCourseId(), age(rng),
                                grade(rng), grade(rng), grade(rng), attendance(rng));
        }

        return result;
    }
}
