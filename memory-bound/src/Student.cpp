#include "Student.h"

#include <utility>

#include "CourseCatalog.h"

Student::Student(int id, std::string name, std::string course, int age,
                 double grade1, double grade2, double grade3, double attendance)
    : Student(id, std::move(name), courses::idOf(course), age,
              grade1, grade2, grade3, attendance)
{
}

Student::Student(int id, std::string name, int courseId, int age,
                 double grade1, double grade2, double grade3, double attendance)
    : id_(id),
      name_(std::move(name)),
      courseId_(courseId),
      age_(age),
      grade1_(grade1),
      grade2_(grade2),
      grade3_(grade3),
      attendance_(attendance)
{
}

int Student::getId() const
{
    return id_;
}

const std::string& Student::getName() const
{
    return name_;
}

const std::string& Student::getCourse() const
{
    return courses::nameOf(courseId_);
}

int Student::getCourseId() const
{
    return courseId_;
}

int Student::getAge() const
{
    return age_;
}

double Student::getGrade1() const
{
    return grade1_;
}

double Student::getGrade2() const
{
    return grade2_;
}

double Student::getGrade3() const
{
    return grade3_;
}

double Student::getAttendance() const
{
    return attendance_;
}

double Student::average() const
{
    double sum = grade1_ + grade2_ + grade3_;
    return sum / 3.0;
}
