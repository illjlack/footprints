/**
* @file socket_server.cpp
* @brief socket 服务封装
* @author liushisheng
* @date 2025-08-15
*/

#include "socket_server.h"
#include "comm/log.h"
#include <format>
#include <cstring>

// 获取最后的错误信息
static std::string getLastErrorMsg()
{
#ifdef _WIN32
    return std::to_string(WSAGetLastError());
#else
    return strerror(errno);
#endif
}

SocketServer::SocketServer(int port) 
    : m_port(port),
    m_listen_fd(INVALID_SOCKET)
#ifdef _WIN32
    , m_wsa_started(false)
#endif
{
}

SocketServer::~SocketServer()
{
    stop();
}

bool SocketServer::init()
{
#ifdef _WIN32
    if (m_wsa_started) return true;
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
        LOG_ERROR(std::format("WSAStartup failed err:{}", err));
        return false;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
    {
        LOG_ERROR(std::format("Could not find a usable version of Winsock.dll, but {}.{} maybe is ok", 
            int(LOBYTE(wsaData.wVersion)), int(HIBYTE(wsaData.wVersion))));
    }

    m_wsa_started = true;
#endif
    return true;
}

bool SocketServer::start()
{
    if (!init()) 
    {
        LOG_ERROR("Failed to initialize socket server");
        return false;
    }

    if (m_listen_fd != INVALID_SOCKET) 
    {
        closeSocket(m_listen_fd);
    }

    m_listen_fd = createSocket();
    if (m_listen_fd == INVALID_SOCKET) 
    {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    LOG_INFO(std::format("Server started successfully on port {}", m_port));
    return true;
}

void SocketServer::stop()
{
    if (m_listen_fd != INVALID_SOCKET)
    {
        closeSocket(m_listen_fd);
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

sock_t SocketServer::accept()
{
    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);

    sock_t clientSocket = ::accept(m_listen_fd, (sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET)
    {
        LOG_ERROR("accept failed");
        return INVALID_SOCKET;
    }

    LOG_INFO(std::format("Client connected: {}:{}, on socket {}", 
        inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientSocket));

    return clientSocket;
}

int SocketServer::recvData(sock_t sock, char* buffer, int len)
{
    int ret = (int)::recv(sock, buffer, len, 0);
    if (ret > 0)
    {
        buffer[ret] = '\0';
        LOG_DEBUG(std::format("received: {}", buffer));
    }
    else if (ret == 0)
    {
        LOG_WARN("Client disconnected");
    }
    else
    {
        LOG_WARN("recv failed");
    }
    return ret;
}

int SocketServer::sendData(sock_t sock, const char* buffer, int len)
{
    int ret = (int)::send(sock, buffer, len, 0);
    if (ret < 0)
    {
        LOG_WARN("send faild");
    }
    return ret;
}

int SocketServer::sendAll(sock_t sock, const char* buffer, int len)
{
    int totalSent = 0;
    while (totalSent < len)
    {
        int ret = sendData(sock, buffer + totalSent, len - totalSent);
        if (ret <= 0)
        {
            return -1;
        }
        totalSent += ret;
    }
    LOG_DEBUG("send succeed");
    return totalSent;
}

// 接收指定长度数据
int SocketServer::recvAll(sock_t sock, char* buffer, int len)
{
    int totalRecv = 0;
    while (totalRecv < len)
    {
        int ret = recvData(sock, buffer + totalRecv, len - totalRecv);
        if (ret <= 0)
        {
            return -1;
        }
        totalRecv += ret;
    }
    buffer[totalRecv] = '\0';
    LOG_DEBUG(std::format("received: {}", buffer));
    return totalRecv;
}



sock_t SocketServer::createSocket()
{
    // 创建一个 IPv4 TCP 套接字
    // AF_INET : IPv4 地址族
    // SOCK_STREAM : 流式套接字，TCP
    // 0 : 协议默认（TCP）
    sock_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
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
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) 
    {
        LOG_WARN(std::format("Failed to bind socket on port {}: {}", m_port, getLastErrorMsg()));
        closeSocket(sock);
        return INVALID_SOCKET;
    }

    // 开始监听客户端连接
    // 128 是最大等待队列长度
    if (listen(sock, 128) < 0) 
    {
        LOG_WARN(std::format("Failed to listen on socket: {}", getLastErrorMsg()));
        closeSocket(sock);
        return INVALID_SOCKET;
    }

	LOG_INFO(std::format("Socket created and listening on port {}", m_port));
    // 成功创建并绑定监听 socket，返回 socket 描述符
    return sock;
}

void SocketServer::closeSocket(sock_t& sock)
{
    if (sock != INVALID_SOCKET)
    {
        LOG_INFO(std::format("Closing socket: {}", sock));
#ifdef _WIN32
        closesocket(sock);
#else
        ::close(sock);
#endif
    }
    sock = INVALID_SOCKET;
}
