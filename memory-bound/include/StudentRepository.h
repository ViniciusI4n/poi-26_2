#pragma once

#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Student.h"

struct CourseStats
{
    std::string course;
    std::size_t count = 0;
    double averageGrade = 0.0;
    double averageAttendance = 0.0;
};

class StudentRepository
{
public:
    explicit StudentRepository(std::size_t historySize);

    void load(std::vector<Student> students);
    void enablePersistence(const std::string& csvPath);

    Student create(const Student& data);
    std::optional<Student> findById(int id) const;
    bool update(int id, const Student& data);
    bool remove(int id);
    std::vector<Student> list(std::size_t offset, std::size_t limit) const;
    std::size_t size() const;

    std::optional<std::vector<double>> history(int id) const;
    std::size_t historySize() const;
    std::size_t historyBytes() const;

    std::vector<Student> searchByName(const std::string& term, std::size_t limit) const;
    std::vector<CourseStats> statistics() const;
    std::vector<Student> topByAverage(std::size_t k) const;

private:
    void saveIfEnabled() const;
    void appendHistory(const Student& student);

    std::string csvPath_;
    std::vector<Student> students_;
    std::unordered_map<int, std::size_t> index_;
    std::size_t historySize_;
    std::vector<double> history_;
    int nextId_ = 1;
    mutable std::shared_mutex mutex_;
};
