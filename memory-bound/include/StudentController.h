#pragma once

#include "StudentRepository.h"

namespace httplib
{
    class Server;
}

class StudentController
{
public:
    explicit StudentController(StudentRepository& repository);

    void registerRoutes(httplib::Server& server);

private:
    StudentRepository& repository_;
};
