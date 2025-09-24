#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>

#define PORT 8080
#define BACKLOG 10
#define BUF_SIZE 4096
#define TP_SLOTS_COUNT 5
#define DATA_LEN 27

/*

  String Data Frame:

  | index | id |  x | y | time |
  |   1   |  5 |  4 | 4 |   13 |

*/
#define DEBUG_FMT "slots[%d]->id: %05d, x: %04d, y: %04d, time: %lld\n"
#define DATA_FMT "%d%05d%04d%04d%lld"

typedef struct {
    int id;
    int x;
    int y;
    long long time; // milisecond
} Slot;

int server_fd, client_fd, touchpad_fd;
struct input_event ev;
char msg[DATA_LEN]={0};
Slot slots[TP_SLOTS_COUNT]={0};

void sigchld_handler(int signo) {
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void setup_tcp(){
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

void setup_toucpad(const char* dev){
    touchpad_fd = open(dev, O_RDONLY);
    if (touchpad_fd < 0) {
        die("touchpad open");
    }

}


// TODO: tcp ve touchpad listener'ı ayır
// TOOD: Dogrudan data gonderme, evet gonder icinde data olsun
// TODO: kullanicilar tcp üzerinden event gönderebilsin
// TODO: keyboard ve mouse device'ları oluştur ve macrolara göre tetikle
int main(void) {
    // TODO: driver tcp ile mevcut device'ları listelesin ve event sectirsin.
    //         cat /proc/bus/input/devices
    const char *dev = "/dev/input/event6";
    setup_tcp();
    setup_toucpad(dev);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t sin_size = sizeof(cli_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &sin_size);
        if (client_fd == -1) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        printf("Yeni bağlantı: %s:%d\n",
               inet_ntoa(cli_addr.sin_addr),
               ntohs(cli_addr.sin_port));

            ssize_t n;

            // JOB LOOP

            int current_slot = 0;
            bool connected = true;
            while(connected){
                if (n = read(touchpad_fd, &ev, sizeof(ev)) > 0) {
                    if (ev.type == EV_ABS) {
                        switch (ev.code) {
                            case ABS_MT_SLOT:
                                current_slot = ev.value;
                                break;
                            case ABS_MT_TRACKING_ID:
                                slots[current_slot].id = ev.value;
                                if (ev.value == -1) {
                                    printf("Slot %d released\n", current_slot);
                                } else {
                                    printf("Slot %d assigned id %d\n", current_slot, ev.value);
                                }
                                break;
                            case ABS_MT_POSITION_X:
                                slots[current_slot].x = ev.value;
                                break;
                            case ABS_MT_POSITION_Y:
                                slots[current_slot].y = ev.value;
                                break;
                            // case ABS_MT_PRESSURE:
                            //     printf("Slot %d pressure = %d\n", current_slot, ev.value);
                            //     break;
                        }
                        slots[current_slot].time = (long long)ev.time.tv_sec * 1000 + ev.time.tv_usec / 1000;
                    } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                        printf("Frame:\n");
                        for (int i=0; i<TP_SLOTS_COUNT; i++) {
                            if (slots[i].id >= 0 || 1==1) {
                                sprintf(msg, DATA_FMT,                                    i,slots[i].id, slots[i].x, slots[i].y, slots[i].time);
                                ssize_t nc = send(client_fd, msg, sizeof(msg), MSG_NOSIGNAL);
                                printf("n: %ld\n",nc );
                                if(nc<0){
                                    perror("Data send error");
                                    connected = false;
                                }
                                printf(DEBUG_FMT,
                                    i,slots[i].id, slots[i].x, slots[i].y, slots[i].time);
                            }
                        }
                    }
                }
            }

            if (n < 0) {
                perror("read");
            }

            printf("Bağlantı kapandı: %s:%d\n",
                   inet_ntoa(cli_addr.sin_addr),
                   ntohs(cli_addr.sin_port));

            close(client_fd);
    }

    close(server_fd);
    return 0;
}
