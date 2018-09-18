#include "headers.h"

bool isTrackerUp=false;

bool notifyTracker=false;

bool recieveChunk=false;

string buff_file="transfer.json";

string tmp_rv_file="sample.json";

string selfIP, trackerIP;
int selfPORT, trackerPORT;

bool SetSocketBlockingEnabled(int fd, bool blocking)
{
   if (fd < 0) return false;
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
}

off_t GetFileLength(string filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) == -1)
        throw runtime_error(strerror(errno));
    return st.st_size;
}

void conTracker(string action) {
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr, my_addr1; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        cout<<"Unable to create the socket!"<<endl;
        return; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(trackerPORT); 

    char tracker_ip[trackerIP.length()+1];
    strcpy(tracker_ip, trackerIP.c_str());
    tracker_ip[trackerIP.length()]=0;
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, tracker_ip, &serv_addr.sin_addr)<=0) { 
        cout<<"Invalid address of another tracker!"<<endl; 
        close(sock);
        return; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        cout<<"Connection with the tracker having ip: "<<trackerIP<<" port: "<<trackerPORT<<" unsuccessfull!"<<endl;
        close(sock);
        return; 
    } else {
        cout<<"Connection with the tracker having ip: "<<trackerIP<<" port: "<<trackerPORT<<" successfull!"<<endl;
        isTrackerUp=true;
    } 
    FILE *fr;
    char transfile[buff_file.length()+1];
    strcpy(transfile, buff_file.c_str());
    transfile[buff_file.length()]=0;
    fr=fopen(transfile,"r");
    unsigned long long int file_len = GetFileLength(buff_file);
    int pkt_size = 1024*62;
    char file_buffer[pkt_size];
    int chunk_id=0;
    int num_chunks=file_len/pkt_size;
    int chunk_size, r, n;
    while(r=fread(file_buffer, sizeof(char), pkt_size, fr)) {
        json j;
        j["agent"]="tracker";
        j["ip"]=selfIP;
        j["action"]="onBootSend";
        j["chunk_id"]=chunk_id;
        if(chunk_id==num_chunks)
            chunk_size=file_len-chunk_id*pkt_size;
        else
            chunk_size=pkt_size;
        j["chunk_size"]=chunk_size;
        j["num_chunks"]=num_chunks+1;
        string strbootData=j.dump();
        char bootData[strbootData.length()+1];
        strcpy(bootData, strbootData.c_str());
        bootData[strbootData.length()]=0;
        send(sock, bootData, strlen(bootData), 0);
        sleep(1);
        cout<<"Sending"<<endl<<file_buffer<<endl<<"::::::::::::::"<<endl;
        n=write(sock, file_buffer, r);
        chunk_id++;
        bzero(file_buffer, pkt_size);
    }
    if(!action.compare("onBootSend")) {
        json jr;
        jr["agent"]="tracker";
        jr["ip"]=selfIP;
        jr["action"]="onBootRequest";
        string reqData=jr.dump();
        char rData[reqData.length()+1];
        strcpy(rData, reqData.c_str());
        rData[reqData.length()]=0;
        cout<<endl<<rData<<":::::"<<endl;
        send(sock, rData, strlen(rData), 0);
    }
    
    while(true);
    // while(true) {
    //     if(isTrackerUp && notifyTracker) {
    //         notifyTracker=false;
    //         notification = json({});
    //         break;
    //     } else if(!isTrackerUp) {
    //         close(sock);
    //     }
    // }
    return;
}

FILE *fw;
void onMessageRecieved(string message) {
    static int c_id, c_size, n_chunks;
    if(!recieveChunk) {
        cout<<message<<endl;
        auto j=json::parse(message);
        string action=j["action"];
        string agent=j["agent"];
        string tip=j["ip"];
        if(!agent.compare("tracker")) {
            if(!tip.compare(trackerIP)) {
                if(!action.compare("onBootSend")) {
                    c_id=j["chunk_id"];
                    c_size = j["chunk_size"];
                    n_chunks = j["num_chunks"];
                    recieveChunk=true;
                    if(fw==NULL) {
                        char rcvfile[tmp_rv_file.length()+1];
                        strcpy(rcvfile, tmp_rv_file.c_str());
                        rcvfile[tmp_rv_file.length()]=0;
                        fw=fopen(rcvfile, "w");
                    }
                    cout<<"About to write chunk: "<<c_id<<endl;
                } else if(!action.compare("onBootRequest")) {
                    cout<<"Initiating thread"<<endl;
                    thread th1(conTracker, "onBootTransfer");
                    th1.detach(); 
                }
            }
        }
    } else {
        char dump_data[c_size];
        for(int i=0;i<c_size;i++)
            dump_data[i]=message[i];
        int n=fwrite(dump_data, sizeof(char), c_size, fw);
        recieveChunk=false;
        if(c_id==n_chunks-1)
            int t=fclose(fw);
        bzero(dump_data, c_size);
        cout<<"Chunk Written, id: "<<c_id<<endl;
    }
}

void listen_for_clients() {
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[1000], max_clients=1000, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    int buffSize=1024*62;
    char buffer[buffSize];
      
    fd_set readfds;
      
    char *message = "ECHO Daemon v1.0 \r\n";
    
    for (i = 0; i < max_clients; i++) 
        client_socket[i] = 0;
    
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
        cout<<"Unable to create the master socket!"<<endl;
    else
        cout<<"Creation of master socket is successfull!"<<endl;
    
    // int flags=fcntl(master_socket, F_GETFL);
    // fcntl(master_socket, F_SETFL, flags | O_NONBLOCK);
    //cout<<SetSocketBlockingEnabled(master_socket, true);

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        cout<<"Unable to allow master socket for multiple connections!"<<endl;
        return;
        // perror("setsockopt");
        // exit(EXIT_FAILURE);
    }
    char self_ip[selfIP.length()+1];
    strcpy(self_ip, selfIP.c_str());
    self_ip[selfIP.length()]=0;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(self_ip);
    address.sin_port = htons(selfPORT);
      
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        cout<<"Binding socket to port: "<<selfPORT<<" failed";
        return;
    } else
        cout<<"Listening on PORT: "<<selfPORT<<endl;


    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
      
    addrlen = sizeof(address);
    cout<<"Waiting for connections!"<<endl;
     
    while(true) 
    {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        
        for(i=0;i<max_clients;i++) {
            sd = client_socket[i];
            if(sd > 0)
                FD_SET(sd, &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }
        activity = select(max_sd+1, &readfds, NULL, NULL, NULL);
          
        //If something happened on the master socket , then its an incoming connection
        if(FD_ISSET(master_socket, &readfds)) {
            if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            for (i=0;i<max_clients;i++) {
                if( client_socket[i] == 0 ) {
                    client_socket[i] = new_socket;
                    cout<<"Client connected successfully, IP ADDRESS: "<<inet_ntoa(address.sin_addr)<<", PORT: "<<ntohs(address.sin_port)<<endl;
                    break;
                }
            }
        }
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds)) {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read(sd, buffer, buffSize)) <= 0) {
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    cout<<"Client disconnected, IP ADDRESS: "<<inet_ntoa(address.sin_addr)<<", PORT: "<<ntohs(address.sin_port)<<endl;
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    cout<<"Message Recieved!"<<endl;
                    //cout<<buffer<<":::::"<<endl;
                    string msg_recieved=buffer;
                    bzero(buffer, buffSize);
                    onMessageRecieved(msg_recieved);
                    cout<<"Returned from thread";
                    //buffer[valread] = '\0';
                    //send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }
}