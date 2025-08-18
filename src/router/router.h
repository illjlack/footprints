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
        auto it = routes.find(req.method + ':' + (req.path.find("/assets/") == std::string::npos ? req.path : "/assets"));
        if (it != routes.end())
        {
            LOG_DEBUG(std::format("Route matched: {} {}", req.method, req.path));
            it->second(req, res);
            return true;
        }
        LOG_WARN(std::format("No route found for: {} {}", req.method, req.path));
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
