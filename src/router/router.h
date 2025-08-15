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

class Router
{
public:
    inline void get(const std::string& path, Handler handler)
    {
        routes["GET:" + path] = handler;
    }

    inline void post(const std::string& path, Handler handler)
    {
        routes["POST:" + path] = handler;
    }

    inline bool route(const HttpRequest& req, HttpResponse& res) const
    {
        auto it = routes.find(req.method + ":" + req.path);
        if (it != routes.end())
        {
            it->second(req, res);
            return true;
        }
        return false;
    }

private:
    std::unordered_map<std::string, Handler> routes;
};

#endif // !ROUTER_H
