#pragma once
#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef _WIN32
#include <WinSock2.h>
#include <mstcpip.h>
#include <ws2def.h>
#pragma comment (lib, "ws2_32.lib")

typedef SOCKET socket_t;
typedef SOCKADDR_IN in_addr_t;
typedef SOCKADDR addr_t;

static void StarupNetwork()
{
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
}

static void CleanupNetwork()
{
    WSACleanup();
}

static void CloseSocket(socket_t sock)
{
    closesocket(sock);
}
#else
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <unistd.h>
#define INVALID_SOCKET -1

typedef int socket_t;
typedef sockaddr_in in_addr_t;
typedef sockaddr addr_t;

static void StarupNetwork()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE,&sa,0);
}

static void CleanupNetwork()
{
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigaction(SIGPIPE,&sa,0);
}

static void CloseSocket(socket_t sock)
{
    close(sock);
}
#endif
#endif