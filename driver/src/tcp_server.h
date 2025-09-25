#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define BACKLOG 10

void setup_tcp_server(void);
void set_client(int new_client_fd);
void disconnect_client(void);
void send_frame_to_client(TouchpadFrame *frame);
void cleanup_tcp_server(void);
int get_server_fd(void);

#endif // TCP_SERVER_H