#include "comm/log.h"
#include "socket/socket_server.h"
#include "http/http_request_parser.h"
#include "http/http_response_builder.h"
#include "router/router.h"
#include "handler/home_handler.h"
#include "handler/assets_handler.h"

size_t getContentLengthFromHeader(const std::string& header_str) 
{
    size_t pos = header_str.find("Content-Length:");
    if (pos != std::string::npos) 
    {
        pos += 15; // 跳过 "Content-Length:"
        // 跳过可能的空格
        while (pos < header_str.size() && isspace(static_cast<unsigned char>(header_str[pos]))) 
        {
            ++pos;
        }
        // 读取数字
        size_t len = 0;
        while (pos < header_str.size() && isdigit(static_cast<unsigned char>(header_str[pos]))) 
        {
            len = len * 10 + (header_str[pos] - '0');
            ++pos;
        }
        return len;
    }
    return 0;
}

int main()
{
    LOG_LEVEL(LogLevel::LOG_DEBUG);

    SocketServer server(8080); // 监听端口
    if (!server.start())
    {
        return -1;
    }

    while (true)
    {
        sock_t client = server.accept();
        if (client != INVALID_SOCKET)
        {
            std::thread([client, &server]()
                {
                    std::string request;
                    char buffer[4096];
                    int n = 0;
                    while ((n = server.recvData(client, buffer, sizeof(buffer) - 1)) > 0)
                    {
                        buffer[n] = '\0';
                        request += buffer;

                        auto headers_end = request.find("\r\n\r\n");
                        if (headers_end != std::string::npos) 
                        {
                            std::string header_str = request.substr(0, headers_end);
                            size_t content_length = getContentLengthFromHeader(header_str);

                            while (request.size() < headers_end + 4 + content_length) 
                            {
                                n = server.recvData(client, buffer, sizeof(buffer) - 1);
                                if (n <= 0) return -1;
                                buffer[n] = '\0';
                                request += buffer;
                            }
                            break;
                        }
                    }

                    auto query =  HttpRequestParser::parse(request);

                    HttpResponse res;
                    try 
                    {
                        if (!Router::getInstance().route(query, res))
                        {
                            res.setStatus(HttpStatus::NotFound);
                            res.setBody("404 Not Found");
                        }
                    } 
                    catch (const std::exception& e) 
                    {
                        LOG_ERROR(std::format("处理请求时发生错误: {}", e.what()));
                        res.setStatus(HttpStatus::InternalServerError);
                        res.setBody("500 Internal Server Error");
                    }

                    std::string responseStr = HttpResponseBuilder::build(res);
                    server.sendAll(client, responseStr.c_str(), responseStr.size());

                    return 0;
                }).detach();
        }
    }

    server.stop();

    return 0;
}
