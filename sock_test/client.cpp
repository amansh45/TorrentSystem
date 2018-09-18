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
#include <map>
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
    my_addr1.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    my_addr1.sin_port = htons(12011);
 
    if (bind(sock, (struct sockaddr*) &my_addr1, sizeof(struct sockaddr_in)) == 0) 
        printf("Binded Correctly\n"); 
    else {
        printf("Unable to bind\n");
        return -1;
    }
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
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
    unsigned long long int file_len=GetFileLength("x.mp4");
    string s;
    int bufferSize=1024*60;
    char buff[bufferSize];
    int num_chunks=file_len/bufferSize;
    int r,n, count=0;
    map<string, string> j;
    while(r=fread(buff, sizeof(char), bufferSize, fr)) {
        string str;
        int chunkSize;
        if(count==num_chunks) {
            chunkSize=file_len-(count*bufferSize);
        } else {
            chunkSize=bufferSize;
        }
        for(int i=0;i<chunkSize;i++)
            str.push_back(buff[i]);
        //cout<<buff<<endl<<":::::::"<<count<<endl;
        string chunk_id=to_string(count);
        string chunk_size=to_string(chunkSize);
        j.insert(pair <string, string>("chunk_id",chunk_id));
        j.insert(pair <string, string>("chunk_size",chunk_size));
        j.insert(pair <string, string>("action", "onBootSend"));
        j.insert(pair <string, string>("ip", "10.1.37.60"));
        j.insert(pair <string, string>("agent", "tracker"));
        j.insert(pair <string, string>("zdata", str));
        map<string, string> :: iterator itr;
        string final_str;
        for(itr=j.begin();itr!=j.end();itr++) {
            string entry;
            string sflag="!$#SRTF|-G#$!";
            string eflag="!$#ENDF|-G#$!";
            entry.append(sflag);
            entry.append(itr->first);
            entry.push_back(':');
            int elen=entry.length();
            string key=itr->first;
            if(!key.compare("zdata")) {
                for(int i=0;i<chunkSize;i++,elen++) 
                    entry.push_back(itr->second[i]);
            } else
                entry.append(itr->second);
            entry.append(eflag);
            final_str.append(entry);
            entry.clear();
        }
        char send_data[final_str.length()+1];
        strcpy(send_data, final_str.c_str());
        send_data[final_str.length()]=0;
        n=write(sock, send_data, r);
        bzero(send_data, final_str.length());
        final_str.clear();
        bzero(buff, bufferSize);
        j.clear();
        count++;
    }
    int t=fclose(fr);
    close(sock);

    return 0; 
    
} 