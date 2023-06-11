#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"
#define TOTAL_HEIRS 8

int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    int total_received = 0;
    int liar_heir = 0;
    int expected_inheritance[TOTAL_HEIRS];
    int received_inheritance[TOTAL_HEIRS];
    int port = DEFAULT_PORT;
    char ip[15] = DEFAULT_IP;

    if (argc == 2) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
    }

    if (argc == 4) {
        liar_heir = atoi(argv[1]);
        if (liar_heir < 0 || liar_heir > TOTAL_HEIRS) {
            printf("Invalid liar heir number. It should be between 0 and %d.\n", TOTAL_HEIRS);
            return -1;
        }
        strcpy(ip, argv[2]);
        port = atoi(argv[3]);
    }

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for monitor\n");
    struct sockaddr_in monitor_address;
    socklen_t monitor_address_len = sizeof(monitor_address);

    char monitor_msg[1];
    if (recvfrom(server_fd, monitor_msg, sizeof(monitor_msg), 0, (struct sockaddr*)&monitor_address, &monitor_address_len) < 0) {
        perror("Monitor accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Monitor connected\n");

    for (int i = 0; i < TOTAL_HEIRS; i++) {
        printf("Waiting for heir %d\n", i + 1);
        struct sockaddr_in heir_address;
        socklen_t heir_address_len = sizeof(heir_address);

        int heir_part;
        if (recvfrom(server_fd, &heir_part, sizeof(heir_part), 0, (struct sockaddr*)&heir_address, &heir_address_len) <= 0) {
            perror("read heir_part failed");
            continue;
        }
        heir_part = ntohl(heir_part);
        expected_inheritance[i] = heir_part;

        if ((i + 1) == liar_heir) {
            heir_part -= 10000;
        }
        received_inheritance[i] = heir_part;
        total_received += heir_part;

        printf("Received inheritance from heir %d\n", i + 1);
        int heir_index = htonl(i + 1);

        if (sendto(server_fd, &heir_index, sizeof(heir_index), 0, (struct sockaddr*)&monitor_address, monitor_address_len) == -1) {
            perror("send heir_index failed");
            continue;
        }

        int heir_part_network = htonl(heir_part);
        if (sendto(server_fd, &heir_part_network, sizeof(heir_part_network), 0, (struct sockaddr*)&monitor_address, monitor_address_len) == -1) {
            perror("send heir_part failed");
            continue;
        }

        int expected_inheritance_network = htonl(expected_inheritance[i]);
        if (sendto(server_fd, &expected_inheritance_network, sizeof(expected_inheritance_network), 0, (struct sockaddr*)&monitor_address, monitor_address_len) == -1) {
            perror("send expected_inheritance failed");
            continue;
        }

        int received_inheritance_network = htonl(received_inheritance[i]);
        if (sendto(server_fd, &received_inheritance_network, sizeof(received_inheritance_network), 0, (struct sockaddr*)&monitor_address, monitor_address_len) == -1) {
            perror("send received_inheritance failed");
            continue;
        }
    }

    printf("Total inheritance received: %d\n", total_received);

    close(server_fd);
    return 0;
}

