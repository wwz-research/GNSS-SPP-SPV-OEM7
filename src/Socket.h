#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "ws2_32.lib")
typedef SOCKET SOCKET_HANDLE;
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <fcntl.h>   /* 供后续可能用到的fcntl调用 */
typedef int  SOCKET_HANDLE;
#   define INVALID_SOCKET  (-1)
#   define SOCKET_ERROR    (-1)
#endif

#include <stdbool.h>
#include <stdio.h>      /* 为perror/printf提供声明 */

/*—————————————  全局对象  —————————————*/
#ifdef _WIN32
static WSADATA wsaData;
static bool    wsaInitialized = false;
#endif

/*—————————————  前置声明  —————————————*/
bool OpenSocket(SOCKET_HANDLE* sock, const char* ip, int port);
void CloseSocket(SOCKET_HANDLE sock);

/*—————————————  跨平台初始化  —————————————*/
void InitializeSocketAPI(void)
{
#ifdef _WIN32
    if (!wsaInitialized)
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            perror("WSAStartup failed");
        wsaInitialized = true;
    }
#endif
}

/*—————————————  建立连接  —————————————*/
bool OpenSocket(SOCKET_HANDLE* sock, const char* ip, int port)
{
    struct sockaddr_in serverAddr;

    InitializeSocketAPI();

#ifdef _WIN32
    * sock = (::socket)(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    * sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (*sock == INVALID_SOCKET)
    {
        perror("Socket creation failed");
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((short)port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0)
    {
        perror("Invalid IP address");
        return false;
    }

    if (connect(*sock,
        (struct sockaddr*)&serverAddr,
        sizeof(serverAddr)) == SOCKET_ERROR)
    {
#ifdef _WIN32
        printf("连接失败，错误代码：%d\n", WSAGetLastError());
        closesocket(*sock);
#else
        perror("Connection failed");
        close(*sock);
#endif
        * sock = INVALID_SOCKET;
        return false;
    }
    return true;
}

/*—————————————  关闭套接字  —————————————*/
void CloseSocket(SOCKET_HANDLE sock)
{
    if (sock == INVALID_SOCKET)
        return;

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

#ifdef _WIN32
    if (wsaInitialized)
    {
        WSACleanup();
        wsaInitialized = false;
    }
#endif
}

#endif /* SOCKET_H */