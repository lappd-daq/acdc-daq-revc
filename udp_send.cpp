#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <IP> <Port> <Message>" << std::endl;
        return 1;
    }

    const char* ip = argv[1];
    int port = std::atoi(argv[2]);
    const char* message = argv[3];

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    ssize_t message_len = std::strlen(message);
    ssize_t bytes_sent = sendto(socket_fd, message, message_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bytes_sent == -1) {
        std::cerr << "Error sending message" << std::endl;
        close(socket_fd);
        return 1;
    }

    std::cout << "Sent " << bytes_sent << " bytes to " << ip << ":" << port << std::endl;

    close(socket_fd);

    return 0;
}
