/**
* @file assets_handler.h
* @brief  静态资源处理
* @author liushisheng
* @date 2025-08-18
*/

#include <router/router.h>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <format>

inline std::string guessMimeType(const std::string& ext)
{
    static const std::unordered_map<std::string, std::string> mime
    {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png" },
        {".jpg", "image/jpeg"},
        {".gif", "image/jpeg"},
        {".svg", "image/gif"},
        {".ico", "image/x-icon"}
    };
    auto it = mime.find(ext);
    if (it != mime.end())return it->second;
    return "application/octet-stream";
}

inline void handlerAssets(const HttpRequest& req, HttpResponse& res) 
{
    static const std::filesystem::path baseDir = std::filesystem::absolute("assets");
    static std::string prefix = "/assets/";

    if (req.path.find(prefix, 0) != 0)
    {
        res.setBody("404 Not Found", "text/plain");
        res.setStatus(HttpStatus::NotFound);
        return;
    }

    std::filesystem::path filePath = std::filesystem::weakly_canonical(baseDir / req.path.substr(prefix.size()));

    if (filePath.string().find(baseDir.string(), 0) != 0 ||
        !std::filesystem::exists(filePath) ||
        !std::filesystem::is_regular_file(filePath))
    {
        res.setBody("403 Forbidden", "text/plain");
        res.setStatus(HttpStatus::Forbidden);
        return;
    }

    std::ifstream ifs(filePath, std::ios::binary);
    std::ostringstream oss;
    oss << ifs.rdbuf();

    res.setBody(oss.str(), guessMimeType(filePath.extension().string()));
    res.setStatus(HttpStatus::OK);
}



static RouteRegister _regAssets("/assets", "GET", handlerAssets);