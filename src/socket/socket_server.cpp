/**
* @file socket_server.cpp
* @brief socket 服务封装
* @author liushisheng
* @date 2025-08-14
*/

#include "socket_server.h"
#include "comm/log.h"
#include <format>

SocketServer::SocketServer(int port) 
    : m_port(port),
    m_listen_fd(INVALID_SOCKET)
{
}

SocketServer::~SocketServer()
{
    stop();
}

bool SocketServer::start()
{
#ifdef _WIN32
    /**
    * int WSAStartup(
    *     [in]  WORD      wVersionRequired,
    *     [out] LPWSADATA lpWSAData
    * );
    * 通过进程启动对 Winsock DLL 的使用
    * 成功返回0
    */
    WSADATA wsaData;
    if (int err = WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        LOG_FATAL(std::format("WSAStartup failed err:{}", err));
        return false;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
    {
        LOG_ERROR(std::format("Could not find a usable version of Winsock.dll, but {}.{} maybe is ok", 
            int(LOBYTE(wsaData.wVersion)), int(HIBYTE(wsaData.wVersion))));
    }

    m_wsa_started = true;
#endif

    if (m_listen_fd != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(m_listen_fd);
#else
        close(m_listen_fd);
#endif
    }
    return true;
}

void SocketServer::stop()
{
    if (m_listen_fd != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(m_listen_fd);
#else
        close(m_listen_fd);
#endif
        m_listen_fd = INVALID_SOCKET;
    }

#ifdef _WIN32
    if (m_wsa_started)
    {
        WSACleanup();
        m_wsa_started = false;
    }
#endif
}

sock_t SocketServer::createSocket()
{
    // 创建一个 IPv4 TCP 套接字
    // AF_INET : IPv4 地址族
    // SOCK_STREAM : 流式套接字，TCP
    // 0 : 协议默认（TCP）
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1; // 创建失败，返回 -1

    // 填充服务器地址信息
    sockaddr_in addr{};
    addr.sin_family = AF_INET;           // 地址族 IPv4
    addr.sin_addr.s_addr = INADDR_ANY;   // 绑定所有可用网卡的 IP
    addr.sin_port = htons(m_port);       // 绑定端口（转换为网络字节序）

    // 设置 socket 选项：允许端口重用
    int opt = 1;
#ifdef _WIN32
    // Windows 下 setsockopt 需要 (const char*) 类型
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    // Linux/Unix 下直接传 int* 就行
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    // 将 socket 绑定到指定的 IP 和端口
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock); // 绑定失败，关闭 socket
#else
        close(sock);
#endif
        return -1; // 返回错误
    }

    // 开始监听客户端连接
    // 128 是最大等待队列长度
    if (listen(sock, 128) < 0) {
#ifdef _WIN32
        closesocket(sock); // 监听失败，关闭 socket
#else
        close(sock);
#endif
        return -1; // 返回错误
    }

    // 成功创建并绑定监听 socket，返回 socket 描述符
    return sock;
}


void SocketServer::close(sock_t sock)
{
    if (sock != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
    }
}
