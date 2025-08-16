/**
* @file home_handler.h
* @brief  首页处理
* @author liushisheng
* @date 2025-08-15
*/

#include <router/router.h>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <format>

// 生成文件名 YYYY-MM-DD-HHMMSS.txt
std::string generateDiaryFilename() 
{
    LOG_DEBUG("Generating new diary filename");
    auto t = std::time(nullptr);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H%M%S") << ".txt";
    return oss.str();
}

// 读取 diaries 文件夹，生成 HTML 列表
std::string buildDiaryListHtml(const std::string& diary_dir) 
{
    LOG_DEBUG(std::format("Building diary list from directory: {}", diary_dir));
    std::ostringstream oss;
    for (auto& p : std::filesystem::directory_iterator(diary_dir))
    {
        if (p.path().extension() == ".txt") 
        {
            std::ifstream ifs(p.path());
            std::string title;
            std::getline(ifs, title); // 简单读取第一行作为标题
            std::string filename = p.path().filename().string();
            oss << "<li>"
                << filename << " - " << title
                << " <a href=\"/diary/" << filename << "\">查看</a>"
                << " <a href=\"/delete/" << filename << "\">删除</a>"
                << "</li>\n";
        }
    }
    return oss.str();
}


void handlerHome(const HttpRequest& req, HttpResponse& res) 
{
    LOG_INFO("Handling home page request");
    // 1. 读取模板
    std::ifstream ifs("html/index.html");
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string html = ss.str();

    // 2. 生成日记列表
    std::string diaries_path = "diaries";
#ifdef DIARIES_PATH
    diaries_path = DIARIES_PATH;
#endif
    std::string diary_list = buildDiaryListHtml(diaries_path);

    // 3. 替换占位符
    size_t pos = html.find("{{DIARY_LIST}}");
    if (pos != std::string::npos)
        html.replace(pos, std::string("{{DIARY_LIST}}").length(), diary_list);

    // 4. 填充 HttpResponse
    res.setBody(html, "text/html");
    res.setStatus(HttpStatus::OK);
    LOG_DEBUG("Home page response prepared successfully");
}

// 静态对象，程序启动时自动执行构造函数注册路由
static RouteRegister _regHome("/", "GET", handlerHome);
