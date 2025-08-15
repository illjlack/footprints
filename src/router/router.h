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

using Handler = std::function<void(const HttpRequest&, HttpResponse&)>;

class RouteRegister 
{
public:
    RouteRegister(const std::string& path, const std::string& method,
                  std::function<void(const HttpRequest&, HttpResponse&)> handler) 
    {
        Router::getInstance().registerRoute(method, path, handler);
    }
};

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
        routes[method + ':' + path] = handler; 
    }

    inline bool route(const HttpRequest& req, HttpResponse& res) const
    {
        auto it = routes.find(req.method + ':' + req.path);
        if (it != routes.end())
        {
            it->second(req, res);
            return true;
        }
        return false;
    }

private:
    Router();
    ~Router();
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    std::unordered_map<std::string, Handler> routes;
};

#endif // !ROUTER_H
