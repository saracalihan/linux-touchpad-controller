#include "common.h"
#include "tcp_server.h"
#include "touchpad_listener.h"
#include "controller.h"
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void cleanup(void) {
    running = false;
    printf("cleanup\n");
    cleanup_tcp_server();
    printf("cleanup_tcp_server\n");
    cleanup_controllers();
    printf("cleanup_controllers\n");
    cleanup_touchpad();
    printf("cleanup_touchpad\n");
    cleanup_event_buffer();
    printf("cleanup_event_buffer\n");
}

void signal_handler(int signo) {
    printf("\nReceived signal %d, shutting down...\n", signo);
    cleanup();
    printf("cleanup done\n");
    exit(0);
}

int main(void) {
    // TODO: driver tcp ile mevcut device'ları listelesin ve event sectirsin.
    //         cat /proc/bus/input/devices
    const char *dev = "/dev/input/event6";
    pthread_t touchpad_thread;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    init_event_buffer();
    init_tcp_server();
    init_controllers();
    init_touchpad(dev);


    if (pthread_create(&touchpad_thread, NULL, touchpad_event_thread, NULL) != 0) {
        perror("pthread_create");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Event listening and TCP server started. Touchpad events will be continuously monitored.\n");

    while (running) {
        struct sockaddr_in cli_addr;
        socklen_t sin_size = sizeof(cli_addr);
        int new_client_fd = accept(get_server_fd(), (struct sockaddr *)&cli_addr, &sin_size);

        if (new_client_fd == -1) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        printf("Nre connection: %s:%d\n",
               inet_ntoa(cli_addr.sin_addr),
               ntohs(cli_addr.sin_port));

        set_client(new_client_fd);
        printf("Client connected and set as active\n");
    }

    pthread_join(touchpad_thread, NULL);

    cleanup();
    return 0;
}