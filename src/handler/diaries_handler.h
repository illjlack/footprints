/**
* @file diaries_handler.h
* @brief  日记处理
* @author liushisheng
* @date 2025-08-15
*/

#include <router/router.h>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <format>

// 生成文件名 YYYY-MM-DD-title.txt
std::string generateDiaryFilename(std::string title) 
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
    oss << std::put_time(&tm, "(%Y-%m-%d)") << title ;
    return oss.str();
}

// 读取 diaries 文件夹，生成 HTML 列表（删除用 POST 表单）
std::string buildDiaryListHtml(const std::string& diary_dir)
{
    std::ostringstream oss;
    oss << "<table>\n";
    oss << "<tr><th>文件名</th><th>操作</th></tr>\n";

    for (auto& p : std::filesystem::directory_iterator(diary_dir))
    {
        std::string filename = p.path().filename().string();

        oss << "<tr>"
            << "<td>" << filename << "</td>"
            << "<td>"
            << "<a href=\"/diary/" << filename << "\">查看</a> "
            << "<a href=\"/delete/" << filename
            << "\" onclick=\"return confirm('确定要删除吗？');\">删除</a>"
            << "</td>"
            << "</tr>\n";
    }

    oss << "</table>\n";
    return oss.str();
}

inline std::unordered_map<std::string, std::string> parseFormData(const std::string& body)
{
    std::unordered_map<std::string, std::string> result;
    std::istringstream ss(body);
    std::string pair;

    while (std::getline(ss, pair, '&'))
    {
        auto pos = pair.find("=");
        if (pos != std::string::npos)
        {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            // url decode
            std::string decoded;
            for (size_t i = 0; i < value.length(); ++i)
            {
                if (value[i] == '+')
                {
                    decoded += ' ';
                }
                else if (value[i] == '%' && i + 2 < value.length())
                {
                    int hex;
                    std::istringstream(value.substr(i + 1, 2)) >> std::hex >> hex;
                    decoded += static_cast<char>(hex);
                    i += 2;
                }
                else
                {
                    decoded += value[i];
                }
            }
            result[key] = decoded;
        }
    }
    return result;
}

std::string utf8_truncate(const std::string& s, size_t chars)
{
    size_t i = 0;
    size_t c = 0;
    while (i < s.size() && c < chars)
    {
        unsigned char byte = s[i];
        size_t step = 0;
        if ((byte & 0x80) == 0x00) step = 1; // ASCII
        else if ((byte & 0xE0) == 0xC0) step = 2;
        else if ((byte & 0xF0) == 0xE0) step = 3;
        else if ((byte & 0xF8) == 0xF0) step = 4;
        else break;

        i += step;
        ++c;
    }
    return s.substr(0, i);
}


void handlerHome(const HttpRequest& req, HttpResponse& res) 
{
    std::ifstream ifs("assets/html/index.html");
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string html = ss.str();

    std::string diaries_path = "diaries";
#ifdef DIARIES_PATH
    diaries_path = DIARIES_PATH;
#endif
    if (!std::filesystem::exists(diaries_path))
    {
        std::filesystem::create_directories(diaries_path);
    }
    std::string diary_list = buildDiaryListHtml(diaries_path);

    size_t pos = html.find("{{DIARY_LIST}}");
    if (pos != std::string::npos)
        html.replace(pos, std::string("{{DIARY_LIST}}").length(), diary_list);

    res.setBody(html, "text/html");
    res.setStatus(HttpStatus::OK);
}

void handlerWrite(const HttpRequest& req, HttpResponse& res)
{
    std::ifstream ifs("assets/html/write.html");
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string html = ss.str();

    res.setBody(html, "text/html");
    res.setStatus(HttpStatus::OK);
}

void handlerPostWrite(const HttpRequest& req, HttpResponse& res)
{
    LOG_INFO(std::format("Raw POST body: {}", req.body));

    auto form =  parseFormData(req.body);
    std::string title = form["title"];
    std::string content = form["content"];

    std::string truncate_string = utf8_truncate(title, 10);
    if (truncate_string.size() < title.size()) truncate_string += "...";

    std::string filename = generateDiaryFilename(truncate_string);

    std::string diaries_path = "diaries";
#ifdef DIARIES_PATH
    diaries_path = DIARIES_PATH;
#endif
    if (!std::filesystem::exists(diaries_path))
    {
        std::filesystem::create_directories(diaries_path);
    }

    std::ofstream ofs(std::filesystem::path(diaries_path) / filename);
    ofs << title << "\n";
    ofs << content << "\n";
    ofs.close();

    res.setStatus(HttpStatus::Found);
    res.headers["Location"] = "/";
    res.setBody("");
}

void handlerViewDiary(const HttpRequest& req, HttpResponse& res)
{
    std::string path = req.path; // "/diary/filename"
    std::string filename = path.substr(std::string("/diary/").size());

    std::string diaries_path = "diaries";
#ifdef DIARIES_PATH
    diaries_path = DIARIES_PATH;
#endif
    std::filesystem::path file_path = std::filesystem::path(diaries_path) / filename;

    if (!std::filesystem::exists(file_path))
    {
        res.setStatus(HttpStatus::NotFound);
        res.setBody("日记不存在");
        return;
    }

    std::ifstream ifs(file_path);
    std::string first_line;
    std::getline(ifs, first_line);  // 第一行作为标题
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string content = ss.str(); // 剩余部分作为正文
    ifs.close();

    // 读取模板
    std::ifstream template_file("assets/html/diary_view.html");
    std::stringstream tss;
    tss << template_file.rdbuf();
    std::string html = tss.str();

    // 替换占位符
    size_t pos = html.find("{{TITLE}}");
    if (pos != std::string::npos)
        html.replace(pos, std::string("{{TITLE}}").length(), first_line);

    pos = html.find("{{TITLE}}");
    if (pos != std::string::npos)
        html.replace(pos, std::string("{{TITLE}}").length(), first_line);

    pos = html.find("{{CONTENT}}");
    if (pos != std::string::npos)
        html.replace(pos, std::string("{{CONTENT}}").length(), content);

    LOG_DEBUG(std::format("html: {}", html));

    res.setBody(html, "text/html");
    res.setStatus(HttpStatus::OK);
}

void handlerDeleteDiary(const HttpRequest& req, HttpResponse& res)
{
    // URL 形式: /delete/filename
    std::string path = req.path; // "/delete/2025-08-18_title"
    std::string filename = path.substr(std::string("/delete/").size());

    std::string diaries_path = "diaries";
#ifdef DIARIES_PATH
    diaries_path = DIARIES_PATH;
#endif
    std::filesystem::path file_path = std::filesystem::path(diaries_path) / filename;

    if (std::filesystem::exists(file_path))
    {
        std::filesystem::remove(file_path);
    }

    // 重定向回首页
    res.setStatus(HttpStatus::Found);
    res.headers["Location"] = "/";
    res.setBody("");
}

// 静态对象，程序启动时自动执行构造函数注册路由
static RouteRegister _reg_home("/", "GET", handlerHome);
static RouteRegister _req_write("/write", "GET", handlerWrite);
static RouteRegister _req_post_write("/post_write", "POST", handlerPostWrite);
static RouteRegister _reg_view("/diary", "GET", handlerViewDiary);
static RouteRegister _reg_delete("/delete", "GET", handlerDeleteDiary);


