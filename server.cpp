#include <iostream>
#include <string>
#include <thread>
#include <mutex> //headere posix pt socketuri
#include <sys/socket.h> //pt functile socket(), bind(), etc...........
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>


#define PORT 8080
#define BUFFER_SIZE 1024

std::mutex cout_mutex; //previne  amestecarea outputului 

//fucntia care ruleaza intr-un thread separat pentru fiecare client
//conn_fd - socketul dedicat acestui client
//clientNo - numarul clientului

void handleClient(int conn_fd, int clientNo){
    //obtinem ip-ul si portul clientului
    sockaddr_in clientInfo, serverInfo;
    socklen_t   addrLen = sizeof(clientInfo);
    //getpeername - adresa capatului celalalt (clientul)
    getpeername (conn_fd,(sockaddr*)&clientInfo, &addrLen);
   
    //getsockname() - adresa capatulului local (serverul, pe acest socket)
    socklen_t serverAddrLen = sizeof(serverInfo);
    getsockname(conn_fd, (sockaddr*)&serverInfo, &serverAddrLen);
    
    //inet_ntop -> transformare binar in string
    char clientIP[INET_ADDRSTRLEN];
    char serverIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN );
    inet_ntop(AF_INET, &serverInfo.sin_addr, serverIP, INET_ADDRSTRLEN );
    
}