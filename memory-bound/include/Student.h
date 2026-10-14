#pragma once

#include <string>

class Student
{
public:
    Student(int id, std::string name, std::string course, int age,
            double grade1, double grade2, double grade3, double attendance);
    Student(int id, std::string name, int courseId, int age,
            double grade1, double grade2, double grade3, double attendance);

    int getId() const;
    const std::string& getName() const;
    const std::string& getCourse() const;
    int getCourseId() const;
    int getAge() const;
    double getGrade1() const;
    double getGrade2() const;
    double getGrade3() const;
    double getAttendance() const;
    double average() const;

private:
    int id_;
    std::string name_;
    int courseId_;
    int age_;
    double grade1_;
    double grade2_;
    double grade3_;
    double attendance_;
};
