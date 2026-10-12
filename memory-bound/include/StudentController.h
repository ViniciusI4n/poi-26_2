#pragma once

#include "Metrics.h"
#include "StudentRepository.h"

namespace httplib
{
    class Server;
}

class StudentController
{
public:
    StudentController(StudentRepository& repository, Metrics& metrics);

    void registerRoutes(httplib::Server& server);

private:
    StudentRepository& repository_;
    Metrics& metrics_;
};
