#include "CourseCatalog.h"

#include <deque>
#include <mutex>
#include <unordered_map>

namespace courses
{
    namespace
    {
        std::mutex mutex;
        std::deque<std::string> names;
        std::unordered_map<std::string, int> ids;
    }

    int idOf(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = ids.find(name);
        if (it != ids.end())
        {
            return it->second;
        }

        int id = static_cast<int>(names.size());
        names.push_back(name);
        ids.emplace(name, id);
        return id;
    }

    const std::string& nameOf(int id)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return names.at(id);
    }

    std::size_t count()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return names.size();
    }
}
