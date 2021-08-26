#include "Socket.h"
#include "RcMem.h"
#include "SystemErr.h"
#include <string.h>
#include <stdio.h>

void HandleClient(void *arg)
{
    //response
    static const char msg[] = "HTTP/1.1 200 OK\r\nServer: chttpd\r\nConnection: close\r\nContent-Length: 11\r\n\r\nHello World";
    //socket
    socket_t sock = (socket_t)arg;
    printf("handle client\n");
    while (1)
    {
        char buf[4096];
        //recv data from sock
        int n = recv(sock,buf,sizeof(buf),0);
        //sock was closed when n == 0
        if (n == 0)
        {
            CloseSocket(sock);
            return;
        }
        //an error was occured when n == -1
        else if (n == -1)
        {
            CloseSocket(sock);
            perror("recv error\n");
            return;
        }
        //ignore data that we recv
        //send msg to sock
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
    //create server socket
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
    //bind 0.0.0.0:8080
    int r = bind(server,(addr_t*)&addr,sizeof(addr));
    if (r == -1)
    {
        //fail to bind 8080
        perror("cannot bind to address\n");
        //close server
        CloseSocket(server);
        CleanupNetwork();
        return -1;
    }
    //now listen on 0.0.0.0:8080
    //backlog is 65535(MAX)
    r = listen(server,65535);
    if (r == -1)
    {
        //fail to listen on 8080
        perror("cannot listen port\n");
        //close server
        CloseSocket(server);
        CleanupNetwork();
        return -1;
    }
    printf("now listen on 0.0.0.0:8080\n");
    while (1)
    {
        //accept new connection
        socket_t sock = accept(server,NULL,0);
        if (sock == INVALID_SOCKET)
        {
            perror("accept error\n");
            continue;
        }
        //thread model: one connection per thread
        //FIXME:inefficient way
        thread_t thr;
        //create thread for new connection
        r = ThreadCreate(&thr,&HandleClient,(void*)sock);
        if (r == -1)
        {
            //fail to create thread
            perror("cannot create thread\n");
            printf("try to use main thread handle\n");
            //handle client in main thread
            HandleClient((void*)sock);
            continue;
        }
        //detach thread
        DetachThread(&thr);
    }
    return 0;
}
