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

    Router router;


    router.get("/hello", [](const HttpRequest& req, HttpResponse& res) {
        res.setBody("Hello, World!", "text/plain");
        });

    while (true)
    {
        sock_t client = server.accept();
        if (client != INVALID_SOCKET)
        {
            std::thread([client, &server, &router]()
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

                    HttpResponse res;
                    if (!router.route(query, res)) 
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
