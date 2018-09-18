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
    int bufferSize=1024*60, n;
    char buff[bufferSize];
    int count=0;
    while(n=read(new_socket, buff, bufferSize)) {
        if(n>=0) {
            int chunk_id, chunk_size, port;
            string action, agent, ip;
            for(int i=0;i<bufferSize;i++) {
                if(buff[i]=='!' && buff[i+1]=='$' && buff[i+2]=='#' && buff[i+3]=='S' && buff[i+4]=='R' && buff[i+5]=='T' && buff[i+6]=='F' && buff[i+7]=='|' && buff[i+8]=='-' && buff[i+9]=='G' && buff[i+10]=='#' && buff[i+11]=='$' && buff[i+12]=='!' ) {
                    i+=13;
                    int j;
                    string value, key;
                    for(j=i;buff[j]!=':';j++)
                        key.push_back(buff[j]);
                    i=j+1;
                    if(key.compare("zdata")) {
                        while(true) {
                            if(buff[i]=='!' && buff[i+1]=='$' && buff[i+2]=='#' && buff[i+3]=='E' && buff[i+4]=='N' && buff[i+5]=='D' && buff[i+6]=='F' && buff[i+7]=='|' && buff[i+8]=='-' && buff[i+9]=='G' && buff[i+10]=='#' && buff[i+11]=='$' && buff[i+12]=='!' )
                                break;
                            else {
                                value.push_back(buff[i]);
                                i++;
                            } 
                        }
                        i--;
                        if(!key.compare("chunk_size"))
                            chunk_size=stoi(value);
                        else if(!key.compare("chunk_id"))
                            chunk_id=stoi(value);
                        else if(!key.compare("ip"))
                            ip.append(value);
                        else if(!key.compare("action"))
                            action.append(value);
                        else if(!key.compare("agent"))
                            agent.append(value);
                        key.clear();
                        value.clear();
                    } else {
                        char dump_buff[chunk_size];
                        int x;
                        for(x=0;x<chunk_size;x++,i++)
                            dump_buff[x]=buff[i];
                        cout<<dump_buff<<endl<<"::::::"<<count<<endl;
                        n=fwrite(dump_buff, sizeof(char), chunk_size, fw);
                        if(n<0) {
                            cout<<"Error in writing chunks to the file.";
                        }
                        bzero(dump_buff, chunk_size);
                        break;
                    }
                    
                } else if(buff[i]=='!' && buff[i+1]=='$' && buff[i+2]=='#' && buff[i+3]=='E' && buff[i+4]=='N' && buff[i+5]=='D' && buff[i+6]=='F' && buff[i+7]=='|' && buff[i+8]=='-' && buff[i+9]=='G' && buff[i+10]=='#' && buff[i+11]=='$' && buff[i+12]=='!' ) {
                    i+=12;
                }
            }
            count++;
            bzero(buff, bufferSize);
            action.clear();
            agent.clear();
            ip.clear();
        }

        //n=fwrite(buff, sizeof(char), bufferSize, fw);
        //bzero(buff, bufferSize);
    }
    int t=fclose(fw);
    close(new_socket);
    return 0; 
} 