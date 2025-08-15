/**
* @file socket_server.h
* @brief socket 服务封装
* @author liushisheng
* @date 2025-08-15
*/

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using sock_t = SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using sock_t = int;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <string>
#include <iostream>

class SocketServer
{
public:
    SocketServer(int port);
    ~SocketServer();

    bool init();
    bool start();
    void stop();

	
    sock_t accept();
    
    int recvData(sock_t sock, char* buffer, int len);
    int sendData(sock_t sock, const char* buffer, int len);

    int recvAll(sock_t sock, char* buffer, int len);
    int sendAll(sock_t sock, const char* buffer, int len);

private:
    sock_t createSocket();
    void closeSocket(sock_t& sock);

private:
    int m_port;
    sock_t m_listen_fd;

#ifdef _WIN32
    bool m_wsa_started;
#endif
};
#endif // SOCKET_SERVER_H
