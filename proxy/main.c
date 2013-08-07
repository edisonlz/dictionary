#include "main.h"
#include "network.h"
#include "process.h"

/* for select */
#include <sys/select.h>

fd_set readset;

void handle_tcp(int client,int remote){
        FD_SET(client, &readset);
        FD_SET(client, &remote);
        while(1){
        
                if (select(client+1, &readset, NULL, NULL, NULL) < 0) {
                    perror("select");
                    return;
                }

//            r, w, e = select.select(fdset, [], [])
//            print r
//            if sock in r:
//                d = sock.recv(4096)
//                print "source", d
//                if remote.send(d) <= 0: break
//            if remote in r:
//                d = remote.recv(4096)
//                print "remote", d
//                if sock.send(d) <= 0: break

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


void echo(int client,char *buf){

    char *char_quit = "quit";
    int quit = strcmp(buf, char_quit);
    printf("quit: %d \n",quit);
     
    if(quit==0){
        *buf = "quit!";
    }
    send_all(client , buf);
    if(quit==0){
        close(client);
    }

}




void process_request(int client, int epoll_fd) {

    ssize_t count;
    char buf[4096];
    
    count = read_all(client, buf);

    printf("read all %s\n",buf);
    
    echo(client, buf);
}




int main(int argc, char** argv) {

    //1.listen socket on port
    int listen_sock = open_non_blocking_socket(9090);

    //2.fork load balance server
    fork_processes(2);

    //3. create epoll and register event
    int epoll_fd = epoll_start(listen_sock);

    //4.io loop for waiting network events.
    io_loop(listen_sock, epoll_fd);

    //no coming
    return 0;
}

