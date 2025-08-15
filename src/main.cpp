#include "comm/log.h"
#include "socket/socket_server.h"
#include "http/http_request_parser.h"
#include "http/http_response_builder.h"
#include "router/router.h"

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
                            size_t content_length = HttpRequestParser::getContentLength(header_str);

                            while (request.size() < headers_end + 4 + content_length) {
                                n = server.recvData(client, buffer, sizeof(buffer) - 1);
                                if (n <= 0) break;
                                buffer[n] = '\0';
                                request += buffer;
                            }
                        }
                    }

                    auto query =  HttpRequestParser::parse(buffer);

                    HttpResponse res;
                    if (!Router::getInstance().route(query, res))
                    {
                        res.setStatus(HttpStatus::NotFound);
                        res.setBody("404 Not Found");
                    }

                    std::string responseStr = HttpResponseBuilder::build(res);

                    server.sendAll(client, responseStr.c_str(), responseStr.size());
                }).detach();
        }
    }

    server.stop();

    return 0;
}
