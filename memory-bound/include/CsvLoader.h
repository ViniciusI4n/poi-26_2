#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Student.h"

namespace csv
{
    std::vector<Student> load(const std::string& path);
    void save(const std::string& path, const std::vector<Student>& students);
    std::vector<Student> scaleUp(const std::vector<Student>& seed, std::size_t target);
}
