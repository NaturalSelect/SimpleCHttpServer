#include "Socket.h"
#include "RcMem.h"
#include "SystemErr.h"
#include <string.h>
#include <stdio.h>

void HandleClient(void *arg)
{
    static const char msg[] = "HTTP/1.1 200 OK\r\nServer: chttpd\r\nConnection: close\r\nContent-Length: 11\r\n\r\nHello World";
    socket_t sock = (socket_t)arg;
    printf("handle client\n");
    while (1)
    {
        char buf[4096];
        int n = recv(sock,buf,sizeof(buf),0);
        if (n == 0)
        {
            CloseSocket(sock);
            return;
        }
        else if (n == -1)
        {
            CloseSocket(sock);
            perror("recv error\n");
            return;
        }
        n = send(sock,msg,sizeof(msg) - 1,0);
        if (n == -1)
        {
            CloseSocket(sock);
            perror("send error\n");
            return;
        }
    }
}

int main(int argc,const char **argv)
{
    StarupNetwork();
    in_addr_t addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
#ifdef IS_WIN
    addr.sin_addr.S_un.S_addr = 0;
#else
    addr.sin_addr.s_addr = 0;
#endif
    socket_t server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        perror("cannot create socket\n");
        CleanupNetwork();
        return -1;
    }
    int r = bind(server,(addr_t*)&addr,sizeof(addr));
    if (r == -1)
    {
        perror("cannot bind to address\n");
        CloseSocket(server);
        CleanupNetwork();
        return -1;
    }
    r = listen(server,65535);
    if (r == -1)
    {
        perror("cannot listen port\n");
        CloseSocket(server);
        CleanupNetwork();
        return -1;
    }
    printf("now listen on 0.0.0.0:8080\n");
    while (1)
    {
        socket_t sock = accept(server,NULL,0);
        if (sock == INVALID_SOCKET)
        {
            perror("accept error\n");
            continue;
        }
        thread_t thr;
        r = ThreadCreate(&thr,&HandleClient,(void*)sock);
        if (r == -1)
        {
            perror("cannot create thread\n");
            printf("try to use main thread handle\n");
            HandleClient((void*)sock);
            continue;
        }
        DetachThread(&thr);
    }
    return 0;
}
