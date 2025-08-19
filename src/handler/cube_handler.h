
/**
* @file cube_handler.h
* @brief  3D 正方体，玩玩
* @author liushisheng
* @date 2025-08-19
*/

#include <router/router.h>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <format>


void handlerCube(const HttpRequest& req, HttpResponse& res)
{
    std::ifstream ifs("assets/html/3D_cube.html");
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string html = ss.str();

    res.setBody(html, "text/html");
    res.setStatus(HttpStatus::OK);
}

static RouteRegister _req_cube("/cube", "GET", handlerCube);
