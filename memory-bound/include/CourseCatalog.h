#pragma once

#include <cstddef>
#include <string>

namespace courses
{
    int idOf(const std::string& name);
    const std::string& nameOf(int id);
    std::size_t count();
}
