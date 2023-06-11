#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define DEFAULT_PORT 8080
#define MONITOR_PORT 8081
#define DEFAULT_IP "127.0.0.1"
#define TOTAL_HEIRS 8
#define MAX_MONITORS 16
#define CONNECTION_REQUEST (-1)

struct sockaddr_in monitor_addresses[MAX_MONITORS];  // array of monitor addresses
int monitor_status[MAX_MONITORS] = {0};  // array of monitor statuses (1 for active, 0 for inactive)

int main(int argc, char const *argv[]) {
    int liar_heir = 0;
    int expected_inheritance[TOTAL_HEIRS];
    int received_inheritance[TOTAL_HEIRS];
    int client_port = DEFAULT_PORT;
    int monitor_port = MONITOR_PORT;
    char ip[15] = DEFAULT_IP;
    struct sockaddr_in client_address, monitor_address;
    socklen_t addrlen = sizeof(client_address);

    if (argc == 2) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
    }

    if (argc == 5) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
        strcpy(ip, argv[2]);
        client_port = atoi(argv[3]);
        monitor_port = atoi(argv[4]);
    }

    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(client_port);

    if (bind(server_fd, (struct sockaddr *) &client_address, sizeof(client_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int monitor_socket = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket
    if (monitor_socket == -1) {
        perror("monitor socket failed");
        exit(EXIT_FAILURE);
    }

    monitor_address.sin_family = AF_INET;
    monitor_address.sin_addr.s_addr = INADDR_ANY;
    monitor_address.sin_port = htons(monitor_port);

    if (bind(monitor_socket, (struct sockaddr *) &monitor_address, sizeof(monitor_address)) < 0) {
        perror("monitor bind failed");
        exit(EXIT_FAILURE);
    }

    // Set monitor socket to non-blocking
    fcntl(monitor_socket, F_SETFL, O_NONBLOCK);

    for (int i = 0; i < TOTAL_HEIRS; i++) {
        printf("Waiting for heir %d\n", i + 1);

        int heir_part;
        if (recvfrom(server_fd, &heir_part, sizeof(heir_part), 0, (struct sockaddr *) &client_address, &addrlen) ==
            -1) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        expected_inheritance[i] = ntohl(heir_part);
        if (i + 1 == liar_heir) {
            received_inheritance[i] = ntohl(heir_part) - 10000;
        } else {
            received_inheritance[i] = ntohl(heir_part);
        }

        printf("Received inheritance from heir %d\n", i + 1);

        // Accept new monitors and update current ones
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(monitor_socket, &read_fds);
        struct timeval tv = {0};
        socklen_t len = sizeof(monitor_address);
        if (select(monitor_socket + 1, &read_fds, NULL, NULL, &tv) > 0) {
            if (FD_ISSET(monitor_socket, &read_fds)) {
                int buf;
                if (recvfrom(monitor_socket, &buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *) &monitor_address,
                             &len) > 0) {
                    buf = ntohl(buf);
                    if (buf == CONNECTION_REQUEST) {
                        for (int j = 0; j < MAX_MONITORS; j++) {
                            if (monitor_status[j] == 0) {
                                monitor_addresses[j] = monitor_address;
                                monitor_status[j] = 1;
                                printf("Monitor %d connected\n", j + 1);
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Send data to all active monitors
        struct {
            int heir_number;
            int received_part;
            int expected_part;
        } network_data;
        network_data.heir_number = htonl(i + 1);
        network_data.received_part = htonl(received_inheritance[i]);
        network_data.expected_part = htonl(expected_inheritance[i]);
        for (int j = 0; j < MAX_MONITORS; j++) {
            if (monitor_status[j] == 1) {
                if (sendto(monitor_socket, &network_data, sizeof(network_data), 0,
                           (struct sockaddr *) &monitor_addresses[j], len) == -1) {
                    printf("Monitor %d disconnected\n", j + 1);
                    monitor_status[j] = 0;
                }
            }
        }
    }

    close(server_fd);
    close(monitor_socket);
    return 0;
}
