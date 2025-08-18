/**
* @file router.h
* @brief  路由注册
* @author liushisheng
* @date 2025-08-15
*/

#ifndef ROUTER_H
#define ROUTER_H

#include "http/http_request_parser.h"
#include "http/http_response_builder.h"
#include <functional>
#include <format>
#include "comm/log.h"

using Handler = std::function<void(const HttpRequest&, HttpResponse&)>;

class Router
{
public:
    inline static Router& getInstance()
    {
        static Router router;
        return router;
    }

    inline void registerRoute(const std::string& method, const std::string& path, Handler handler)
    {
        LOG_INFO(std::format("Registering route: {} {}", method, path));
        routes[method + ':' + path] = handler; 
    }

    inline bool route(const HttpRequest& req, HttpResponse& res) const
    {
        std::string key = req.method + ':';
        std::string path = req.path;

        for (std::string current = path; !current.empty();)
        {
            auto it = routes.find(key + current);
            if(it != routes.end())
            {
                it->second(req, res);
                return true;
            }

            auto pos = current.find_last_of('/');
            if(pos == std::string::npos || pos == 0)
            {
                break;
            }
            current = current.substr(0, pos);
        }

        auto it = routes.find(key + '/');
        if(it != routes.end())
        {
            it->second(req, res);
            return true;
        }

        return false;
    }

private:
    Router() = default;
    ~Router() = default;
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    std::unordered_map<std::string, Handler> routes;
};

class RouteRegister
{
public:
    RouteRegister(const std::string& path, const std::string& method,
        std::function<void(const HttpRequest&, HttpResponse&)> handler)
    {
        Router::getInstance().registerRoute(method, path, handler);
    }
};

#endif // !ROUTER_H
