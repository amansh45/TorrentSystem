// Client side C/C++ program to demonstrate Socket programming 
#include <unistd.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <iostream>
#include <string.h> 
#include "json.hpp"
#include <fstream>
#include <sys/stat.h>
#define PORT 9254 
using namespace std;
using json = nlohmann::json;   

off_t GetFileLength(string filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st) == -1)
        throw runtime_error(strerror(errno));
    return st.st_size;
}

int main(int argc, char const *argv[]) 
{ 
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr, my_addr1; 
    char *hello = "Hello from client";
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 

    my_addr1.sin_family = AF_INET; 
    my_addr1.sin_addr.s_addr = inet_addr("10.2.129.89"); 
    my_addr1.sin_port = htons(12011);
 
    if (bind(sock, (struct sockaddr*) &my_addr1, sizeof(struct sockaddr_in)) == 0) 
        printf("Binded Correctly\n"); 
    else {
        printf("Unable to bind\n");
        return -1;
    }
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "10.2.129.89", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    
    FILE *fr;
    fr=fopen("x.mp4","r");
    string s;
    int bufferSize=1024*512;
    char buff[bufferSize];
    int r,n;
    json j;
    while(r=fread(buff, sizeof(char), bufferSize, fr)) {
        string str=buff;
        j["sample"]=str;
        string json_data = j.dump();
        char send_data[json_data.length()+1];
        strcpy(send_data, json_data.c_str());
        send_data[json_data.length()]=0;
        n=write(sock, buff, r);
        bzero(buff, bufferSize);
    }
    int t=fclose(fr);
    close(sock);











    // ifstream target("x.mp4", ifstream::in | ifstream::binary);
    // ofstream new_torrent;
    // new_torrent.open("t.mp4", std::ios_base::app);
    // int bufferSize = 1024*512;

    // if(!target.is_open())
    // {
    //     return -1;
    // }
    // char buffer[bufferSize];
    // int count=0;
    // do {
    //     target.read(buffer, bufferSize);
    //     new_torrent.write(buffer, bufferSize);
    //     send(sock, buffer, strlen(buffer), 0);
    //     cout<<buffer<<endl<<"::::::::::::::::::::::::::::::::::::::::::"<<count<<endl;
    //     count++;
    //     //bzero((char *)buffer, 1024*512);
    //     memset(buffer, 0, sizeof(buffer));
    //     sleep(2);
    // } while(target);

    close(sock);
    
    return 0; 
    
} 