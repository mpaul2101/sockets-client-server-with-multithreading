#include <iostream>
#include <string>
#include <thread>
#include <mutex> //headere posix pt socketuri
#include <sys/socket.h> //pt functile socket(), bind(), etc...........
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>


#define PORT 9090
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

    //nthos() network byte order, host byte order
    int clientPort = ntohs(clientInfo.sin_port);
    int serverPort = ntohs(serverInfo.sin_port);   // = 8080

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[+] Client " << clientNo << " conectat!\n";
        std::cout << "    IP client: " << clientIP << " | Port client: " << clientPort << "\n";
        std::cout << "    IP server: " << serverIP << " | Port server: " << serverPort << "\n";
    }
    
    //Bucla de comunicare:
    char buffer[BUFFER_SIZE];
    while(true){
        memset(buffer, 0 ,BUFFER_SIZE);
        //recv() - primim date, returneaza nr. de bytes, 0 = client deconectat, -1 = eroare
        int bytesReceived = recv(conn_fd, buffer, BUFFER_SIZE -1,0);

        if(bytesReceived <=0 ){
                 std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "\n[-] Client " << clientNo << " (" 
                      << clientIP << ":" << clientPort << ") s-a deconectat.\n";
            break;
        }
        std::string mesaj(buffer,bytesReceived);
        //afisam pe server în formatul cerut
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "\n[SERVER] Mesaj de la Client " << clientNo 
                      << ": \"" << mesaj << "\"\n";
            std::cout << "  Adresa IP sursa:      " << clientIP   << "\n";
            std::cout << "  Adresa IP destinatie: " << serverIP   << "\n";
            std::cout << "  Port sursa:           " << clientPort << "\n";
            std::cout << "  Port destinatie:      " << serverPort << "\n";
        }

        //construim si trimitem raspunsul spre client
        std::string raspuns =
            "[SERVER -> CLIENT " + std::to_string(clientNo) + "] \"" + mesaj + "\"\n"
            "  Adresa IP sursa:      " + serverIP                    + "\n"
            "  Adresa IP destinatie: " + clientIP                    + "\n"
            "  Port sursa:           " + std::to_string(serverPort)  + "\n"
            "  Port destinatie:      " + std::to_string(clientPort)  + "\n";

        send(conn_fd, raspuns.c_str(), raspuns.size(), 0);
    }

    close(conn_fd);  //pe Linux: close(), nu closesocket()
}

int main(){
    std::cout<<"Server Pornit";
    std::cout<<"Port:"<<PORT<<"\n\n";
    //socket() - creeam socketul de ascultare
    //AF_INET = IPv4
    //SOCK_STREAM = TCP
    //0 = protocol implicit
    int listen_fd =socket(AF_INET, SOCK_STREAM,0);
    if (listen_fd < 0){
        std::cout<<"SOCKET() ERROR";
        return 1;
    }
    std::cout<< "SOCKET creat (fd = "<< listen_fd<< ")\n";
    int opt = 1;
    //permitem reutilizarea portului
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

//bind()- asociem socketul la port
 sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(PORT);   // htons = host→network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY;    // acceptă pe orice interfață

    if (bind(listen_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Eroare bind()");
        close(listen_fd);

        return 1;
    }
    std::cout << "bind() pe portul " << PORT << "\n";
//listen() — intram în mod ascultare
    // 16 = dimensiunea cozii de conexiuni pending

    if (listen(listen_fd, 16) < 0) {
        perror("Eroare listen()");
        close(listen_fd);
        return 1;
    }
    std::cout << "listen() activ. Astept clienți...\n\n";

    //BUCLA PRINCIPALA — acceptam clienti si cream thread pentru fiecare
    int clientCounter = 0;

    while (true) {
        sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);

        //accept() — blocheaza pana vine un client
        //returneaza un NOU fd dedicat clientului respectiv
        //listen_fd ramane liber sa accepte alti clienti în paralel
        
        int conn_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);
        if (conn_fd < 0) {
            perror("Eroare accept()");
            continue;
        }

        clientCounter++;

        //cream thread separat pentru acest client
        //detach() = thread-ul rulează independent
        std::thread t(handleClient, conn_fd, clientCounter);
        t.detach();
    }

    close(listen_fd);
    return 0;
}