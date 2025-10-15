#include "tcp_server.h"
#include "controller.h"
#include "common.h"

static int server_fd = -1;
static int client_fd = -1;
static pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t reader_thread;
static pthread_t sender_thread;

void sigchld_handler(int signo) {
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void init_tcp_server(void) {
    struct sockaddr_in serv_addr;
    int yes = 1;

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        die("sigaction");
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        die("socket");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        die("setsockopt");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        die("bind");
    }

    if (listen(server_fd, BACKLOG) == -1) {
        die("listen");
    }

    printf("TCP sunucu: port %d dinleniyor...\n", PORT);
}

void* tcp_reader_thread(void* arg) {
    int fd = *(int*)arg;
    printf("tcp reader starting for fd: %d\n", fd);
    char data[RECV_DATA_LEN] = {0};
    while(1){
        if(fd == -1){
            sleep(1);
            continue;
        }

        bool msg_recv_done = false;
        bool first_time = true;
        int str_size =0, total_bytes_recived=0;
        do{
            ssize_t bytes_received = 0;
            char buff[RECV_DATA_LEN] = {0};
            bytes_received = recv(fd, buff, sizeof(char)*RECV_DATA_LEN, 0);
            if(bytes_received == 0) {
                printf("Client disconnected (recv returned 0)\n");
                client_fd = -1;
                return NULL;
            } else if(bytes_received < 0) {
                perror("recv error");
                break;
            }
            total_bytes_recived += bytes_received;
            // if this is first package, parse 'size'
            if(first_time){
                const char* format = "%1d%4d%[^\n]";
                int controller; // UNUSED
                char* v; // UNUSED
                int ps = sscanf(buff, format, &controller, &str_size, &v);

                // prepare for next connection
                if(ps != 3){
                    printf("[RECV ERROR]: parsing failed: '%s'\n", buff);
                    msg_recv_done = false;
                    first_time = true;
                    str_size =0;
                    total_bytes_recived=0;
                    continue;
                }

                first_time = false;
                strcpy(data, buff);
                if(bytes_received>str_size){
                    data[str_size+1+4]= '\0';   
                    msg_recv_done= true;
                }
            } else {
                strncat(data, buff, RECV_DATA_LEN - strlen(data) - 1);
                if(total_bytes_recived>=str_size){
                    msg_recv_done= true;
                    data[str_size+1+4]= '\0';
                }
            }

        } while(!msg_recv_done);

        printf("[RECV DATA]: %s\n", data);
        exec_str_command(data);
        memset(data, 0, RECV_DATA_LEN); // Buffer'ı temizle
    }
    printf("tcp reader finished for fd: %d\n", fd);
    return NULL;
}

void* tcp_sender_thread(void* arg) {
    int fd = *(int*)arg;
    printf("tcp sender starting for fd: %d\n", fd);
    TouchpadFrame current_frame;
    while(1){
        get_frame_from_buffer(&current_frame);
        send_frame_to_client(&current_frame);
    }
    printf("tcp sender finished for fd: %d\n", fd);
    return NULL;
}

void set_client(int new_client_fd) {
    pthread_mutex_lock(&client_mutex);
    if (client_fd != -1) {
        printf("Replacing existing client connection\n");
        close(client_fd);

        pthread_cancel(reader_thread);
        pthread_join(reader_thread, NULL);

        pthread_cancel(sender_thread);
        pthread_join(sender_thread, NULL);
    }
    client_fd = new_client_fd;
    printf("Setting new client_fd: %d\n", client_fd);

    if (pthread_create(&reader_thread, NULL, tcp_reader_thread, &client_fd) != 0) {
        perror("pthread_create tcp_reader_thread failed");
        close(client_fd);
        client_fd = -1;
    }
    if (pthread_create(&sender_thread, NULL, tcp_sender_thread, &client_fd) != 0) {
        perror("pthread_create tcp_sender_thread failed");
        close(client_fd);
        client_fd = -1;
    }
    pthread_mutex_unlock(&client_mutex);
}

int get_client(){
    return client_fd;
}

void disconnect_client(void) {
    if (client_fd != -1) {
        close(client_fd);
        client_fd = -1;
        printf("Client disconnected\n");
    }
}

void send_frame_to_client(TouchpadFrame *frame) {
    pthread_mutex_lock(&client_mutex);
    if (client_fd == -1) {
        printf("[TCP SEND ERROR]: client fd is -1\n");
        pthread_mutex_unlock(&client_mutex);
        return;
    }

    char messages[TP_SLOTS_COUNT][DATA_LEN];
    int message_count = 0;

    // Prepare slots
    for (int i = 0; i < TP_SLOTS_COUNT; i++) {
        if (SEND_ALL_SLOTS || frame->slots[i].id >= 0) {
            sprintf(messages[message_count], DATA_FMT,
                    i, frame->slots[i].id, frame->slots[i].x,
                    frame->slots[i].y, frame->slots[i].time);
            message_count++;
        }
    }

    for (int j = 0; j < message_count; j++) {
        ssize_t sent = send(client_fd, messages[j], DATA_LEN, MSG_NOSIGNAL);
        if (sent < 0) {
            printf("Client send error, disconnecting\n");
            close(client_fd);
            client_fd = -1;
            pthread_mutex_unlock(&client_mutex);
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void cleanup_tcp_server(void) {
    printf("cleanup_tcp_server starting...\n");
    pthread_cancel(reader_thread);
    pthread_join(reader_thread, NULL);
    if (client_fd != -1) {
        disconnect_client();
    }

    if (server_fd > 0) {
        close(server_fd);
        server_fd = -1;
    }
    pthread_mutex_destroy(&client_mutex);
    printf("cleanup_tcp_server done\n");
}

int get_server_fd(void) {
    return server_fd;
}