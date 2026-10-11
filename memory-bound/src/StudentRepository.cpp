#include "StudentRepository.h"

#include <algorithm>
#include <functional>
#include <mutex>
#include <queue>
#include <utility>

#include "CsvLoader.h"

void StudentRepository::load(std::vector<Student> students)
{
    std::unique_lock lock(mutex_);

    students_ = std::move(students);
    index_.clear();
    index_.reserve(students_.size());
    nextId_ = 1;

    for (std::size_t i = 0; i < students_.size(); i++)
    {
        int id = students_[i].getId();
        index_[id] = i;

        if (id >= nextId_)
        {
            nextId_ = id + 1;
        }
    }
}

void StudentRepository::enablePersistence(const std::string& csvPath)
{
    std::unique_lock lock(mutex_);
    csvPath_ = csvPath;
}

void StudentRepository::saveIfEnabled() const
{
    if (csvPath_.empty())
    {
        return;
    }

    csv::save(csvPath_, students_);
}

Student StudentRepository::create(const Student& data)
{
    std::unique_lock lock(mutex_);

    students_.emplace_back(nextId_, data.getName(), data.getCourse(), data.getAge(),
                           data.getGrade1(), data.getGrade2(), data.getGrade3(),
                           data.getAttendance());
    index_[nextId_] = students_.size() - 1;
    nextId_++;

    saveIfEnabled();
    return students_.back();
}

std::optional<Student> StudentRepository::findById(int id) const
{
    std::shared_lock lock(mutex_);

    auto it = index_.find(id);
    if (it == index_.end())
    {
        return std::nullopt;
    }

    return students_[it->second];
}

bool StudentRepository::update(int id, const Student& data)
{
    std::unique_lock lock(mutex_);

    auto it = index_.find(id);
    if (it == index_.end())
    {
        return false;
    }

    students_[it->second] = Student(id, data.getName(), data.getCourse(), data.getAge(),
                                    data.getGrade1(), data.getGrade2(), data.getGrade3(),
                                    data.getAttendance());

    saveIfEnabled();
    return true;
}

bool StudentRepository::remove(int id)
{
    std::unique_lock lock(mutex_);

    auto it = index_.find(id);
    if (it == index_.end())
    {
        return false;
    }

    std::size_t pos = it->second;
    std::size_t last = students_.size() - 1;

    if (pos != last)
    {
        students_[pos] = std::move(students_[last]);
        index_[students_[pos].getId()] = pos;
    }

    students_.pop_back();
    index_.erase(id);

    saveIfEnabled();
    return true;
}

std::vector<Student> StudentRepository::list(std::size_t offset, std::size_t limit) const
{
    std::shared_lock lock(mutex_);

    std::vector<Student> page;
    for (std::size_t i = offset; i < students_.size(); i++)
    {
        if (page.size() >= limit)
        {
            break;
        }
        page.push_back(students_[i]);
    }

    return page;
}

std::size_t StudentRepository::size() const
{
    std::shared_lock lock(mutex_);
    return students_.size();
}

std::vector<Student> StudentRepository::searchByName(const std::string& term,
                                                     std::size_t limit) const
{
    std::shared_lock lock(mutex_);

    std::vector<Student> matches;
    for (const Student& student : students_)
    {
        if (student.getName().find(term) == std::string::npos)
        {
            continue;
        }

        matches.push_back(student);
        if (matches.size() >= limit)
        {
            break;
        }
    }

    return matches;
}

std::vector<CourseStats> StudentRepository::statistics() const
{
    std::shared_lock lock(mutex_);

    std::unordered_map<std::string, CourseStats> byCourse;
    for (const Student& student : students_)
    {
        CourseStats& stats = byCourse[student.getCourse()];
        stats.count++;
        stats.averageGrade += student.average();
        stats.averageAttendance += student.getAttendance();
    }

    std::vector<CourseStats> result;
    for (auto& entry : byCourse)
    {
        CourseStats stats = entry.second;
        stats.course = entry.first;
        stats.averageGrade = stats.averageGrade / stats.count;
        stats.averageAttendance = stats.averageAttendance / stats.count;
        result.push_back(stats);
    }

    std::sort(result.begin(), result.end(),
              [](const CourseStats& a, const CourseStats& b)
              {
                  return a.course < b.course;
              });

    return result;
}

std::vector<Student> StudentRepository::topByAverage(std::size_t k) const
{
    std::shared_lock lock(mutex_);

    typedef std::pair<double, std::size_t> Entry;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> heap;

    for (std::size_t i = 0; i < students_.size(); i++)
    {
        double avg = students_[i].average();

        if (heap.size() < k)
        {
            heap.push(Entry(avg, i));
        }
        else if (!heap.empty() && avg > heap.top().first)
        {
            heap.pop();
            heap.push(Entry(avg, i));
        }
    }

    std::vector<Student> result;
    while (!heap.empty())
    {
        result.push_back(students_[heap.top().second]);
        heap.pop();
    }

    std::reverse(result.begin(), result.end());
    return result;
}
