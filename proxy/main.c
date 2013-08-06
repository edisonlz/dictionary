#include "main.h"
#include "network.h"
#include "process.h"



void io_loop(int listen_sock, int epoll_fd) {

    int nfds;
    uint32_t events;
    struct epoll_event epoll_events[MAX_EVENTS];
    
    while(1) {
    
        nfds = epoll_wait(epollfd, epoll_events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; ++ i) {
            events = epoll_events[i].events;
            if (epoll_events[i].data.fd == listen_sock) {
                
                accept_incoming(listen_sock, epoll_events[i]);

            }  else {

                if ((events & EPOLLERR)) {
                        #ifdef DEBUG
                           printf("error condiction, events: %d, fd: %d\n", events, epoll_fd);
                        #endif
                    
                        close_and_clean(epoll_fd);
                    
                } else {
                    if (events & EPOLLIN) {
                    
                        #ifdef DEBUG
                           printf("process request, sock_fd %d\n", epoll_fd);
                        #endif

                        process_request(epoll_fd);
                    }
                    
                    if (events & EPOLLOUT) {
                            #ifdef DEBUG
                                printf("EPOLLOUT sock_fd: %d write\n",epoll_fd);
                            #endif
                    }
                }
            }
        }
    }
}


void echo(int epoll_fd,char *buf){
    send_all(epoll_fd , buf);
}

void process_request(int epoll_fd) {

    ssize_t count;
    char buf[4096];
    
    count = read_all(epoll_fd, &buf);
    
    echo(epoll_fd, &buf);
}


int main(int argc, char** argv) {

    struct epoll_event ev;
    int listen_sock, efd;

    listen_sock = open_nonb_listenfd(9090);

    //fork load balance server
    fork_processes(2);
    
    efd = epoll_create(100);
    if (efd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
     }

    ev.events = EPOLLIN;        // read
    ev.data.fd = listen_sock;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    
    io_loop(listen_sock, efd);
    return 0;
}
