#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define SERVER_IP   "127.0.0.1"   // localhost — serverul e pe același PC
#define SERVER_PORT 9090
#define BUFFER_SIZE 1024

int main() {
    std::cout << "   CLIENT   \n";
    std::cout << "Conectare la " << SERVER_IP << ":" << SERVER_PORT << "\n\n";

    //socket()
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Eroare socket()");
        return 1;
    }

    //pregatim adresa serverului
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port   = htons(SERVER_PORT);

    //inet_pton = "presentation to network" — string → binar
    if (inet_pton(AF_INET, SERVER_IP, &address.sin_addr) <= 0) {
        perror("Adresa IP invalida");
        close(client_fd);
        return 1;
    }

    //connect() — ne conectam la server
    if (connect(client_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("Eroare connect() — serverul e pornit?");
        close(client_fd);
        return 1;
    }
    std::cout << "[OK] Conectat la server!\n";

    //afisam portul local asignat automat de OS
    sockaddr_in localAddr;
    socklen_t localLen = sizeof(localAddr);
    getsockname(client_fd, (sockaddr*)&localAddr, &localLen);
    std::cout << "Portul tau local (asignat de OS): " 
              << ntohs(localAddr.sin_port) << "\n\n";

    //Bucla de trimitere 
    char buffer[BUFFER_SIZE];
    std::string mesaj;

    while (true) {
        std::cout << "Tu: ";
        std::getline(std::cin, mesaj);

        if (mesaj == "exit" || mesaj == "quit") break;
        if (mesaj.empty()) continue;

        //trimitem mesajul
        send(client_fd, mesaj.c_str(), mesaj.size(), 0);

        //primim raspunsul de la server
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived <= 0) {
            std::cout << "Serverul a inchis conexiunea.\n";
            break;
        }

        std::cout << "\n" << std::string(buffer, bytesReceived) << "\n";
    }

    close(client_fd);
    return 0;
}