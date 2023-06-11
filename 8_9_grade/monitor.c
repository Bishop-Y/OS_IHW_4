#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEFAULT_PORT 8081
#define DEFAULT_IP "127.0.0.1"
#define CONNECTION_REQUEST (-1)

int main(int argc, char const *argv[]) {
    char ip[15] = DEFAULT_IP;
    int port = DEFAULT_PORT;

    if (argc == 3) {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
    }

    int monitor_fd = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket
    if (monitor_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_address.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    int connection_request = htonl(CONNECTION_REQUEST);
    if (sendto(monitor_fd, &connection_request, sizeof(connection_request), 0, (struct sockaddr *) &server_address,
               sizeof(server_address)) == -1) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        socklen_t len = sizeof(server_address);
        struct {
            int heir_number;
            int received_part;
            int expected_part;
        } network_data;

        if (recvfrom(monitor_fd, &network_data, sizeof(network_data), MSG_WAITALL, (struct sockaddr *) &server_address,
                     &len) == -1) {
            perror("recv failed");
            exit(EXIT_FAILURE);
        }
        int expected_inheritance = ntohl(network_data.expected_part);
        int heir_number = ntohl(network_data.heir_number);
        int received_inheritance = ntohl(network_data.received_part);
        if (expected_inheritance != received_inheritance) {
            printf("Heir %d: expected %d, received %d. Lawyer lied on this heir!\n", heir_number, expected_inheritance,
                   received_inheritance);
        } else {
            printf("Heir %d: expected %d, received %d.\n", heir_number, expected_inheritance, received_inheritance);
        }
        if (heir_number == 8) {
            break;
        }
    }

    close(monitor_fd);
    return 0;
}
