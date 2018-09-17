// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <iostream>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <string.h>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;  
using namespace std; 
#define PORT 9254 



int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char *hello = "Hello from server"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }

    FILE *fw;
    fw=fopen("sample.mp4", "w");
    int bufferSize=1024*512, n;
    char buff[bufferSize];
    while(n=read(new_socket, buff, bufferSize)) {
        string buffstr = buff;
        //auto j=json::parse(buffstr);
        string new_buff=j["sample"];
        char writebuff[new_buff.length()+1];
        strcpy(writebuff, new_buff.c_str());
        writebuff[new_buff.length()]=0;
        if(n<0)
            cout<<"read failed!"<<endl;
        n=fwrite(writebuff, sizeof(char), n, fw);
        if(n<0)
            cout<<"Write Failed!"<<endl;
        bzero(writebuff, bufferSize);
    }
    int t=fclose(fw);
    close(new_socket);
    // ofstream new_torrent;
    // new_torrent.open("sample.mp4", std::ios_base::app);

    // int cid=0;
    // char buffer[1024*512];
    // while(true) { 
    
    //     valread = read(new_socket , buffer, 1024*512);
    //     // auto j=json::parse(buffer);
    //     // int chunk_id=j["chunk_id"];
    //     // unsigned long long int file_size = j["file_size"];
    //     // if(chunk_id < (file_size/(1024*512))) {
    //     //     new_torrent<<j["chunk_data"];
    //     // } else if(chunk_id == (file_size/(1024*512))) {
    //     //     int chunk_size = file_size-(chunk_id*1024*512);
    //     //     char data[chunk_size+1];
    //     //     string jdata = j["chunk_data"];
    //     //     for(int i=0;i<chunk_size;i++) {
                
    //     //         data[i]=jdata[i];
    //     //     }
    //     //     data[chunk_size]=0;
    //     //     new_torrent<<data;
    //     //     new_torrent.close();
    //     // }
    //     new_torrent.write(buffer, 1024*512);
    //     cout<<buffer<<endl<<":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"<<cid<<endl;
    //     cid++;
    //     memset(buffer, 0, sizeof(buffer));
    //     //bzero((char *)buffer, 1024*512);
    //     if(cid==15)
    //         break;       
    // }
    // new_torrent.close();
    return 0; 
} 