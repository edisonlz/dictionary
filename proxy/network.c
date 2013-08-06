#include "network.h"


void make_socket_non_blocking(int sfd) {
    int flags;
    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
       perror("fcntl");
       exit(1);
    }
    flags |= O_NONBLOCK;
    if(fcntl(sfd, F_SETFL, flags) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}


int open_nonb_listenfd(int port) {
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    make_socket_non_blocking(listenfd);
    
    if (listen(listenfd, LISTENQ) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    return listenfd;
}


void accept_incoming(int listen_sock, int epoll_fd){

    struct epoll_event ev;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof clientaddr;
    
    client = accept(listen_sock,  (struct sockaddr *) &clientaddr, &clientlen);
    if(client < 0){
        perror("accept");
        return;
    }

    #ifdef DEBUG
        printf("accept %s:%d, sock_fd is %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), conn_sock);
    #endif
    
    make_socket_non_blocking(client);
    
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = client;
    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, client, &ev) < 0) {
        fprintf(stderr, "epoll set insertion error: fd=%d", client);
        exit(EXIT_FAILURE);
    }
}


void close_and_clean(int epollfd) {
    close(epollfd);
}

int read_all(int fd,char *buf){

    ssize_t n = 0;
    ssize_t count;
    while ((count = read(fd, buf + n, sizeof buf)) > 0) {
        n += count;
    }
}


int send_all(int fd,char *buf){

    ssize_t nwrite, data_size = strlen(buf);
    ssize_t n = data_size;
    while (n > 0) {

        nwrite = write(fd, buf + data_size - n, n);
        if (nwrite < n) {
            if (nwrite == -1 && errno != EAGAIN) {
                perror("write error");
                return -1;
            }
            break;
        }
        n -= nwrite;
    }
   return 0;
}