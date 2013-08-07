#include "main.h"
#include "network.h"
#include "process.h"

/* for select */
#include <sys/select.h>



void handle_tcp(int client,int remote){

        fd_set readset;

        FD_ZERO(&readset);
        
        FD_SET(client, &readset);
        FD_SET(remote, &readset);
        

        char buf[4096];
        while(1){
        
                if (select(client+1, &readset, NULL, NULL, NULL) < 0) {
                    perror("select");
                    return;
                }
                
                 /* Process all of the fds that are still set in readset */
                for (int i=0; i < 2; i++) {

                    if (FD_ISSET(client, &readset)) {
                        read_all(client, buf);
                        printf("client:%d,remore:%d, %s", client,remote,buf);
                        send_all(remote, buf);
                        *buf = '\0';
                    }
                    else if(FD_ISSET(remote, &readset)){
                        read_all(remote, buf);
                        printf("remore:%d,client:%d, %s", remote, client, buf);
                        send_all(client, buf);
                        *buf = '\0';
                    }
                }
        }
}



void io_loop(int listen_sock, int epoll_fd) {

    int nfds;
    uint32_t events;
    struct epoll_event epoll_events[MAX_EVENTS];
    
    while(1) {
    
        nfds = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; ++ i) {
            events = epoll_events[i].events;
            if (epoll_events[i].data.fd == listen_sock) {
                
                accept_incoming(listen_sock, epoll_fd);

            }  else {

                if ((events & EPOLLERR)) {
                        #ifdef DEBUG
                           printf("error condiction, events: %d, fd: %d\n", events, epoll_fd);
                        #endif
                    
                        close_and_clean(epoll_events[i].data.fd);
                    
                } else {
                    if (events & EPOLLIN) {

                        printf("process request, sock_fd %d\n", epoll_fd);
                        process_request(epoll_events[i].data.fd, epoll_fd);
                        
                    }
                    
                    if (events & EPOLLOUT) {
                         printf("EPOLLOUT sock_fd: %d write\n",epoll_fd);
                    }
                }
            }
        }
    }
}




int connect_remote(char *server,int port){

    int remote = socket(AF_INET, SOCK_STREAM, 0);

    //Co
    struct sockaddr_in sin;
    struct in_addr addr;
    addr.s_addr =
    

    sin.sin_family = AF_INET;
    sin.sin_port = htons(9090);
    inet_pton(AF_INET,"10.103.13.18",&sin.sin_addr);
    
    if (connect(remote, (struct sockaddr*) &sin, sizeof(sin))) {
        perror("connect");
        close(remote);
        return 1;
    }
    return remote;
}


void process_request(int client, int epoll_fd) {

    ssize_t count;
    char buf[4096];
    
    count = read_all(client, buf);
    
    printf("read all %s\n",buf);

    int remote = connect_remote("",80);
    
    handle_tcp(client,remote);
}




int main(int argc, char** argv) {

    //1.listen socket on port
    int listen_sock = open_non_blocking_socket(20000);

    //2.fork load balance server
    fork_processes(2);

    //3. create epoll and register event
    int epoll_fd = epoll_start(listen_sock);

    //4.io loop for waiting network events.
    io_loop(listen_sock, epoll_fd);

    //no coming
    return 0;
}

