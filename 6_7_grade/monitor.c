#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"

int main(int argc, char const *argv[]) {
    int monitor_fd;
    struct sockaddr_in serv_addr;
    int port = DEFAULT_PORT;
    char ip[15] = DEFAULT_IP;

    if (argc >= 3) {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
    }

    if ((monitor_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Send empty message to connect to server
    char monitor_msg[1]; // empty message for protocol
    if (sendto(monitor_fd, monitor_msg, sizeof(monitor_msg), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("send monitor_msg failed");
        exit(EXIT_FAILURE);
    }

    // Receive data from server
    while (1) {
        int heir_index;
        if (recvfrom(monitor_fd, &heir_index, sizeof(heir_index), 0, NULL, NULL) <= 0) {
            break;
        }
        heir_index = ntohl(heir_index);

        int heir_part;
        if (recvfrom(monitor_fd, &heir_part, sizeof(heir_part), 0, NULL, NULL) <= 0) {
            break;
        }
        heir_part = ntohl(heir_part);

        int expected_inheritance;
        if (recvfrom(monitor_fd, &expected_inheritance, sizeof(expected_inheritance), 0, NULL, NULL) <= 0) {
            break;
        }
        expected_inheritance = ntohl(expected_inheritance);

        int received_inheritance;
        if (recvfrom(monitor_fd, &received_inheritance, sizeof(received_inheritance), 0, NULL, NULL) <= 0) {
            break;
        }
        received_inheritance = ntohl(received_inheritance);

        printf("Heir %d, declared part: %d, expected: %d, received: %d. ", heir_index, heir_part, expected_inheritance, received_inheritance);
        if (expected_inheritance != received_inheritance) {
            printf("Lawyer lied on this heir!");
        }
        printf("\n");
        if (heir_index == 8) {
            break;
        }
    }

    close(monitor_fd);
    return 0;
}
