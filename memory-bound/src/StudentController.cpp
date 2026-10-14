#include "StudentController.h"

#include <chrono>
#include <stdexcept>
#include <string>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

namespace
{
    json toJson(const Student& s)
    {
        json j;
        j["id"] = s.getId();
        j["name"] = s.getName();
        j["course"] = s.getCourse();
        j["age"] = s.getAge();
        j["grade1"] = s.getGrade1();
        j["grade2"] = s.getGrade2();
        j["grade3"] = s.getGrade3();
        j["attendance"] = s.getAttendance();
        j["average"] = s.average();
        return j;
    }

    json toJson(const std::vector<Student>& students)
    {
        json array = json::array();
        for (const Student& s : students)
        {
            array.push_back(toJson(s));
        }
        return array;
    }

    json toJson(const CourseStats& c)
    {
        json j;
        j["course"] = c.course;
        j["count"] = c.count;
        j["averageGrade"] = c.averageGrade;
        j["averageAttendance"] = c.averageAttendance;
        return j;
    }

    void checkText(const std::string& text, const std::string& field)
    {
        if (text.empty() || text.find_first_of(",\r\n") != std::string::npos)
        {
            throw std::invalid_argument(field + " must not be empty or contain commas/line breaks");
        }
    }

    Student fromJson(const json& body)
    {
        std::string name = body.at("name").get<std::string>();
        std::string course = body.at("course").get<std::string>();
        checkText(name, "name");
        checkText(course, "course");

        return Student(0,
                       name,
                       course,
                       body.at("age").get<int>(),
                       body.at("grade1").get<double>(),
                       body.at("grade2").get<double>(),
                       body.at("grade3").get<double>(),
                       body.at("attendance").get<double>());
    }

    void sendJson(httplib::Response& res, const json& body, int status = 200)
    {
        res.status = status;
        res.set_content(body.dump(), "application/json");
    }

    void sendError(httplib::Response& res, int status, const std::string& message)
    {
        json body;
        body["error"] = message;
        sendJson(res, body, status);
    }

    std::size_t queryNumber(const httplib::Request& req, const std::string& key,
                            std::size_t fallback)
    {
        if (!req.has_param(key))
        {
            return fallback;
        }

        try
        {
            return std::stoul(req.get_param_value(key));
        }
        catch (const std::exception&)
        {
            return fallback;
        }
    }

    httplib::Server::Handler timed(Metrics& metrics, std::string method, std::string route,
                                   httplib::Server::Handler handler)
    {
        return [&metrics, method, route, handler](const httplib::Request& req,
                                                  httplib::Response& res)
        {
            auto start = std::chrono::steady_clock::now();
            auto elapsed = [&start]()
            {
                return std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
                    .count();
            };

            try
            {
                handler(req, res);
            }
            catch (...)
            {
                metrics.recordRequest(method, route, 500, elapsed());
                throw;
            }
            metrics.recordRequest(method, route, res.status, elapsed());
        };
    }
}

StudentController::StudentController(StudentRepository& repository, Metrics& metrics)
    : repository_(repository),
      metrics_(metrics)
{
}

void StudentController::registerRoutes(httplib::Server& server)
{
    server.Get("/health", timed(metrics_, "GET", "/health",
        [this](const httplib::Request&, httplib::Response& res)
    {
        json body;
        body["status"] = "ok";
        body["students"] = repository_.size();
        sendJson(res, body);
    }));

    server.Get("/students", timed(metrics_, "GET", "/students",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        std::size_t offset = queryNumber(req, "offset", 0);
        std::size_t limit = queryNumber(req, "limit", 20);
        sendJson(res, toJson(repository_.list(offset, limit)));
    }));

    server.Get(R"(/students/(\d+))", timed(metrics_, "GET", "/students/{id}",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        int id = std::stoi(req.matches[1]);
        std::optional<Student> student = repository_.findById(id);

        if (!student.has_value())
        {
            sendError(res, 404, "student not found");
            return;
        }

        sendJson(res, toJson(student.value()));
    }));

    server.Get(R"(/students/(\d+)/history)", timed(metrics_, "GET", "/students/{id}/history",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        int id = std::stoi(req.matches[1]);
        std::optional<std::vector<double>> history = repository_.history(id);

        if (!history.has_value())
        {
            sendError(res, 404, "student not found");
            return;
        }

        json body;
        body["id"] = id;
        body["history"] = history.value();
        sendJson(res, body);
    }));

    server.Post("/students", timed(metrics_, "POST", "/students",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        try
        {
            Student created = repository_.create(fromJson(json::parse(req.body)));
            sendJson(res, toJson(created), 201);
        }
        catch (const json::exception& e)
        {
            sendError(res, 400, std::string("invalid body: ") + e.what());
        }
        catch (const std::invalid_argument& e)
        {
            sendError(res, 400, e.what());
        }
        catch (const std::exception& e)
        {
            sendError(res, 500, e.what());
        }
    }));

    server.Put(R"(/students/(\d+))", timed(metrics_, "PUT", "/students/{id}",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        try
        {
            int id = std::stoi(req.matches[1]);
            Student data = fromJson(json::parse(req.body));

            if (!repository_.update(id, data))
            {
                sendError(res, 404, "student not found");
                return;
            }

            sendJson(res, toJson(repository_.findById(id).value()));
        }
        catch (const json::exception& e)
        {
            sendError(res, 400, std::string("invalid body: ") + e.what());
        }
        catch (const std::invalid_argument& e)
        {
            sendError(res, 400, e.what());
        }
        catch (const std::exception& e)
        {
            sendError(res, 500, e.what());
        }
    }));

    server.Delete(R"(/students/(\d+))", timed(metrics_, "DELETE", "/students/{id}",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        try
        {
            int id = std::stoi(req.matches[1]);

            if (!repository_.remove(id))
            {
                sendError(res, 404, "student not found");
                return;
            }

            res.status = 204;
        }
        catch (const std::exception& e)
        {
            sendError(res, 500, e.what());
        }
    }));

    server.Get("/students/search", timed(metrics_, "GET", "/students/search",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        if (!req.has_param("name"))
        {
            sendError(res, 400, "missing query param: name");
            return;
        }

        std::string term = req.get_param_value("name");
        std::size_t limit = queryNumber(req, "limit", 20);
        sendJson(res, toJson(repository_.searchByName(term, limit)));
    }));

    server.Get("/students/stats", timed(metrics_, "GET", "/students/stats",
        [this](const httplib::Request&, httplib::Response& res)
    {
        json courses = json::array();
        for (const CourseStats& c : repository_.statistics())
        {
            courses.push_back(toJson(c));
        }

        json body;
        body["total"] = repository_.size();
        body["courses"] = courses;
        sendJson(res, body);
    }));

    server.Get("/students/ranking", timed(metrics_, "GET", "/students/ranking",
        [this](const httplib::Request& req, httplib::Response& res)
    {
        std::size_t top = queryNumber(req, "top", 10);
        sendJson(res, toJson(repository_.topByAverage(top)));
    }));

    server.Get("/metrics", [this](const httplib::Request&, httplib::Response& res)
    {
        res.set_content(metrics_.render(), "text/plain; version=0.0.4; charset=utf-8");
    });
}
