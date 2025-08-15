#include "comm/log.h"
#include "socket/socket_server.h"
#include "http/http_request_parser.h"
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

                        if (request.find("\r\n\r\n") != std::string::npos) // HTTP头结束
                            break;
                    }

                    auto query =  HttpRequestParser::parse(buffer);

                    LOG_DEBUG(request);

                    if (n > 0)
                    {
                        std::cout << "Received request:\n" << buffer << "\n";

                        // 构造 HTTP 响应
                        std::string html = R"(<!DOCTYPE html>
                            <html>
                            <head><title>Test Page</title></head>
                            <body>
                            <h1>Hello from SocketServer!</h1>
                            <p>This is a simple HTML page.</p>
                            </body>
                            </html>)";

                        std::string response =
                            (std::string)"HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=UTF-8\r\n" +
                            "Content-Length:" + std::to_string(html.size()) + "\r\n\r\n" +
                            html;

                        server.sendAll(client, response.c_str(), response.size());
                    }
                }).detach();
        }
    }

    server.stop();

    return 0;
}
