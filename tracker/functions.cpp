#include "headers.h"

bool isTrackerUp=false;

bool notifyTracker=false;

bool recieveChunk=false;

bool sendData=false;

string buff_file="transfer.json";

string tmp_rv_file="sample.json";

string selfIP, trackerIP, seedFilePath;

int selfPORT, trackerPORT, selfSendingPort, trackerSendingPort;

string sharable_data;

json seedlist, sharelist;

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

bool is_file_exist(char *fileName) {
    std::ifstream infile(fileName);
    bool var=infile.good();
    infile.close();
    return var;
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

    char self_ip[selfIP.length()+1];
    strcpy(self_ip, selfIP.c_str());
    self_ip[selfIP.length()]=0;

    my_addr1.sin_family = AF_INET; 
    my_addr1.sin_addr.s_addr = inet_addr(self_ip); 
    my_addr1.sin_port = htons(selfSendingPort);
 
    if (bind(sock, (struct sockaddr*) &my_addr1, sizeof(struct sockaddr_in)) == 0) 
        cout<<"Binded Correctly with sending port as: "<<selfSendingPort<<endl; 
    else {
        cout<<"Unable to bind the port!";
        return;
    }

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
    

    if(is_file_exist(transfile)) {
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
            j["port"]=selfPORT;
            string strbootData=j.dump();
            char bootData[strbootData.length()+1];
            strcpy(bootData, strbootData.c_str());
            bootData[strbootData.length()]=0;
            send(sock, bootData, strlen(bootData), 0);
            sleep(2);
            n=write(sock, file_buffer, r);
            sendData=false;
            chunk_id++;
            bzero(file_buffer, pkt_size);
            bzero(bootData, strbootData.length()+1);
        }
        if( remove(transfile) != 0 )
            cout<<"Error in removing the File: "<<transfile<<endl;
        else
            cout<<"File: "<<transfile<<" removed successfully!"<<endl;    
    }
    
    if(!action.compare("onBootSend")) {
        json jr;
        jr["agent"]="tracker";
        jr["ip"]=selfIP;
        jr["action"]="onBootRequest";
        jr["port"]=selfPORT;
        string reqData=jr.dump();
        char rData[reqData.length()+1];
        strcpy(rData, reqData.c_str());
        rData[reqData.length()]=0;
        //cout<<endl<<rData<<":::::"<<endl;
        send(sock, rData, strlen(rData), 0);
    }
    
    while(true) {
        if(isTrackerUp && notifyTracker) {
            char tobeShared[sharable_data.length()+1];
            strcpy(tobeShared, sharable_data.c_str());
            tobeShared[sharable_data.length()]=0;
            send(sock, tobeShared, strlen(tobeShared), 0);
            sharable_data.clear();
            notifyTracker=false;
        } else if(!isTrackerUp) {
            close(sock);
            break;
        }
    }
    return;
}

string retrieve_seeds(string hash_o_hash) {
    json::iterator it;
    json get_details;
    bool found=false;
    get_details["data"]["hash_o_hash"]=hash_o_hash;
    get_details["agent"]="tracker";
    get_details["ip"]=selfIP;
    get_details["port"]=selfPORT;
    get_details["action"]="getawk";
    get_details["data"]["seeders"]=json::array();

    for(it = seedlist["seederlist"].begin(); it!=seedlist["seederlist"].end(); ++it) {
        json entry=*it;
        string sample_hash=entry["hash"];
        if(!sample_hash.compare(hash_o_hash)) {
            json j=json::array();
            for(json::iterator itr=entry["seeders"].begin();itr!=entry["seeders"].end();itr++) {
                json single_seed=*itr;
                j.push_back(single_seed);
            }
            found=true;
            get_details["data"]["seeders"]=j;
            break;
        }
    }

    string return_msg=get_details.dump();
    return return_msg;
}

void conClient(string Ip, int Port, string message) {
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr, my_addr1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout<<"Error in creation of the socket!"<<endl;
        return; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(Port); 

    char self_ip_addr[selfIP.length()+1];
    strcpy(self_ip_addr, selfIP.c_str());
    self_ip_addr[selfIP.length()]=0;
  
    char client_ip_addr[Ip.length()+1];
    strcpy(client_ip_addr, Ip.c_str());
    client_ip_addr[Ip.length()]=0;

    if(inet_pton(AF_INET, client_ip_addr, &serv_addr.sin_addr)<=0) { 
        cout<<"Invalid address of the client, IP: "<<Ip<<" PORT: "<<Port<<endl;
        return; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        cout<<"Unable to connect with the client, IP: "<<Ip<<" PORT: "<<Port<<endl; 
        return; 
    } else {
        cout<<"Connection successfully established, about to send data to the client!"<<endl;
    }
    char tobeShared[message.length()+1];
    strcpy(tobeShared, message.c_str());
    tobeShared[message.length()]=0;
    send(sock, tobeShared, strlen(tobeShared), 0);
    close(sock);
}

void shallWrite(json data) {
    json::iterator it;
    bool hash_found=false, seed_found=false;
    int count=0;
    for(it = seedlist["seederlist"].begin(); it != seedlist["seederlist"].end(); ++it) {
        json entry=*it;
        string stored_hash=entry["hash"];
        if(!stored_hash.compare(data["hash"])) {
            hash_found=true;
            string s_ip=data["client"]["ip"];
            int s_port=data["client"]["port"];
            json::iterator itr;
            seed_found=false;
            for(itr=entry["seeders"].begin(); itr!=entry["seeders"].end(); ++itr) {
                json sub_entry=*itr;
                if(!s_ip.compare(sub_entry["ip"]) && s_port==sub_entry["port"]) {
                    seed_found=true;
                    break;
                }
            }
            if(!seed_found) {
                json seed_data;
                seed_data["ip"]=s_ip;
                seed_data["port"]=s_port;
                entry["seeders"].push_back(seed_data);
            }
            *it=entry;
            break;
        }
    }
    if(!hash_found)
        seedlist["seederlist"].push_back(data);
    if(!hash_found || !seed_found) {
        string dump_data=seedlist.dump();
        ofstream seeding_file(seedFilePath);
        seeding_file<<dump_data;
        seeding_file.close();
    }
}

void shallRemove(string hash) {
    json::iterator it;
    bool hash_found=false;
    for(it = seedlist["seederlist"].begin(); it != seedlist["seederlist"].end(); ++it) {
        json entry=*it;
        string stored_hash=entry["hash"];
        if(!stored_hash.compare(hash)) {
            hash_found=true;
            break;
        }
    }
    if(hash_found) {
        seedlist["seederlist"].erase(it);
        cout<<seedlist;
        string dump_data=seedlist.dump();
        ofstream seeding_file(seedFilePath);
        seeding_file<<dump_data;
        seeding_file.close();
    }
}

void shallRemoveForTransfer(json hash) {
    cout<<"10"<<endl;
    json::iterator itr;
    cout<<"9"<<endl;
    bool flag=false, remove_flag=false;
    cout<<"8"<<endl;
    for(itr=sharelist["share_data"].begin();itr!=sharelist["share_data"].end();++itr) {
        json jtarget=*itr;
        string target=jtarget.dump();
        cout<<"7"<<endl;
        if(!target.compare(hash.dump())) {
            flag=true;
            return;
        } else {
            target.clear();
            cout<<"6"<<endl;
            target=jtarget["data"]["hash"];
            cout<<"5"<<endl;
            string dtype=jtarget["data_type"];
            cout<<"99"<<endl;
            string comingHash=hash["data"]["hash"];
            cout<<comingHash<<":::"<<target<<endl;
            if(!target.compare(comingHash) && !dtype.compare("share")) {
                cout<<"4"<<endl;
                remove_flag=true;
                break;
            }
        }
    }
    if(remove_flag) {
        cout<<"3"<<endl;
        sharelist["share_data"].erase(itr);
    }
    if(!flag) {
        cout<<"2"<<endl;
        sharelist["share_data"].push_back(hash);
        cout<<"1"<<endl;
        string dump_data=sharelist.dump();
        ofstream seeding_file(buff_file);
        seeding_file<<dump_data;
        seeding_file.close();
    }
}

void shallWriteForTransfer(json Shared) {
    bool flag=false, seed_found=false, seed_added=false;
    json::iterator itr;
    string true_identity=Shared["data"]["seeders"].dump();
    for(itr=sharelist["share_data"].begin();itr!=sharelist["share_data"].end();++itr) {
        json jtarget=*itr;
        string target=jtarget.dump();
        if(!target.compare(Shared.dump())) {
            flag=true;
            return;
        } else {
            target.clear();
            target=jtarget["data"]["hash"];
            string type=jtarget["data_type"];
            if(!target.compare(Shared["data"]["hash"]) && !type.compare(Shared["data_type"])) {
                string s_ip=Shared["data"]["client"]["ip"];
                int s_port=Shared["data"]["client"]["port"];
                json::iterator it;
                for(it=jtarget["data"]["seeders"].begin(); it!=jtarget["data"]["seeders"].end(); ++it) {
                    json sub_entry=*it;
                    if(!s_ip.compare(sub_entry["ip"]) && s_port==sub_entry["port"]) {
                        seed_found=true;
                        break;
                    }
                }
                if(!seed_found) {
                    json seed_data;
                    seed_data["ip"]=s_ip;
                    seed_data["port"]=s_port;
                    jtarget["data"]["seeders"].push_back(seed_data);
                    seed_added=true;
                    *itr=jtarget;                
                    break;
                }
            }
        }
    }
    if(!flag || !seed_found) {
        if(!seed_added && !seed_found)
            sharelist["share_data"].push_back(Shared);
        string dump_data=sharelist.dump();
        ofstream seeding_file(buff_file);
        seeding_file<<dump_data;
        seeding_file.close();
    }
}

void shallAddForTransfer(json Shared) {
    bool seed_found=false, seed_added=false;
    json::iterator itr;
    string true_identity=Shared["data"]["client"].dump();
    for(itr=sharelist["share_data"].begin();itr!=sharelist["share_data"].end();++itr) {
        json jtarget=*itr;
        string target=jtarget["data"]["hash"];
        string type=jtarget["data_type"];
        if(!target.compare(Shared["data"]["hash"]) && !type.compare(Shared["data_type"])) {
            string s_ip=Shared["data"]["client"]["ip"];
            int s_port=Shared["data"]["client"]["port"];
            json::iterator it;
            for(it=jtarget["data"]["seeders"].begin(); it!=jtarget["data"]["seeders"].end(); ++it) {
                json sub_entry=*it;
                if(!s_ip.compare(sub_entry["ip"]) && s_port==sub_entry["port"]) {
                    seed_found=true;
                    break;
                }
            }
            if(!seed_found) {
                json seed_data;
                seed_data["ip"]=s_ip;
                seed_data["port"]=s_port;
                jtarget["data"]["seeders"].push_back(seed_data);
                seed_added=true;
            }
            *itr=jtarget;
            break;
        }
    }
    if(seed_added) {
        string dump_data=sharelist.dump();
        ofstream seeding_file(buff_file);
        seeding_file<<dump_data;
        seeding_file.close();
    }
}

void shallAddSeed(json data) {
    json::iterator it;
    bool hash_found=false, seed_found=false;
    int count=0;
    for(it = seedlist["seederlist"].begin(); it != seedlist["seederlist"].end(); ++it) {
        json entry=*it;
        string stored_hash=entry["hash"];
        if(!stored_hash.compare(data["hash"])) {
            hash_found=true;
            for(json::iterator itt=data["seeders"].begin(); itt!=data["seeders"].end(); ++itt) {
                string s_ip=data["seeders"]["ip"];
                int s_port=data["seeders"]["port"];
                json::iterator itr;
                seed_found=false;
                for(itr=entry["seeders"].begin(); itr!=entry["seeders"].end(); ++itr) {
                    json sub_entry=*itr;
                    if(!s_ip.compare(sub_entry["ip"]) && s_port==sub_entry["port"]) {
                        seed_found=true;
                        break;
                    }
                }
                if(!seed_found) {
                    json seed_data;
                    seed_data["ip"]=s_ip;
                    seed_data["port"]=s_port;
                    entry["seeders"].push_back(seed_data);
                }
            }
            *it=entry;
            break;
        }
    }
    if(!hash_found)
        seedlist["seederlist"].push_back(data);
    if(!hash_found || !seed_found) {
        string dump_data=seedlist.dump();
        ofstream seeding_file(seedFilePath);
        seeding_file<<dump_data;
        seeding_file.close();
    }
}


void process_file() {
    int counting=1;
    json j;
    ifstream transrecievelist(tmp_rv_file);
    transrecievelist>>j;
    json::iterator itr;
    for(itr=j["share_data"].begin();itr!=j["share_data"].end();++itr) {
        json element = *itr;
        string dtype=element["data_type"];
        if(!dtype.compare("share")) {
            shallWrite(element["data"]);
        } else if(!dtype.compare("remove")) {
            shallRemove(element["data"]["hash"]);
        }
    }
    transrecievelist.close();
}


FILE *fw;
void onMessageRecieved(string message) {
    static int c_id, c_size, n_chunks;
    if(!recieveChunk) {
        auto j=json::parse(message);
        string action=j["action"];
        string agent=j["agent"];
        string tip=j["ip"];
        int tport=j["port"];
        if(!agent.compare("tracker")) {
            if(!tip.compare(trackerIP)) {
                isTrackerUp=true;
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
                } else if(!action.compare("onBootRequest")) {
                    cout<<"Initiating thread"<<endl;
                    thread th1(conTracker, "onBootTransfer");
                    th1.detach(); 
                } else if(!action.compare("onShare")) {
                    auto rec_data=json::parse(message);
                    shallWrite(rec_data["data"]);
                } else if(!action.compare("onRemove")) {
                    auto rec_data=json::parse(message);
                    string testing_str=rec_data["hash_o_hash"];
                    shallRemove(testing_str);
                } else if(!action.compare("onAddSeed")) {
                    auto rec_data = json::parse(message);
                    shallWrite(rec_data["data"]);
                }
            }
        } else if(!agent.compare("client")) {
            if(!action.compare("share")) {
                json du_data;
                du_data["agent"]="tracker";
                du_data["action"]="onShare";
                du_data["ip"]=selfIP;
                du_data["port"]=selfPORT;
                du_data["data"]=j["data"];
                sharable_data=du_data.dump();
                if(isTrackerUp) {
                    shallWrite(du_data["data"]);
                    notifyTracker=true;
                } else {
                    shallWrite(du_data["data"]);
                    json tobeShared;
                    tobeShared["data_type"]="share";
                    tobeShared["data"]=du_data["data"];
                    shallWriteForTransfer(tobeShared);   
                }
            } else if(!action.compare("get")) {
                string hash_o_hash=j["hash_o_hash"];
                string seedList=retrieve_seeds(hash_o_hash);
                cout<<seedList<<endl;
                conClient(tip, tport, seedList);
            } else if(!action.compare("remove")) {
                json du_data;
                string hash_o_hash = j["hash_o_hash"];
                du_data["agent"]="tracker";
                du_data["action"]="onRemove";
                du_data["ip"]=selfIP;
                du_data["port"]=selfPORT;
                du_data["hash_o_hash"]=hash_o_hash;
                sharable_data=du_data.dump();
                if(isTrackerUp) {
                    shallRemove(hash_o_hash);
                    notifyTracker=true;
                } else {
                    shallRemove(hash_o_hash);
                    json tobeRemoved;
                    tobeRemoved["data_type"]="remove";
                    tobeRemoved["data"]["hash"]=hash_o_hash;
                    shallRemoveForTransfer(tobeRemoved);
                }
            } else if(!action.compare("addSeed")) {
                json du_data;
                du_data["agent"]="tracker";
                du_data["action"]="onAddSeed";
                du_data["ip"]=selfIP;
                du_data["port"]=selfPORT;
                du_data["data"]["client"]["ip"]=tip;
                du_data["data"]["client"]["port"]=tport;
                du_data["data"]["hash"]=j["hash_o_hash"];
                sharable_data=du_data.dump();
                if(isTrackerUp) {
                    shallWrite(du_data["data"]);
                    notifyTracker=true;
                } else {
                    shallWrite(du_data["data"]);
                    json tobeShared;
                    tobeShared["data_type"]="share";
                    tobeShared["data"]=du_data["data"];
                    shallAddForTransfer(tobeShared);
                }
            }
        }
    } else {
        char dump_data[c_size];
        for(int i=0;i<c_size;i++)
            dump_data[i]=message[i];
        int n=fwrite(dump_data, sizeof(char), c_size, fw);
        recieveChunk=false;
        if(c_id==n_chunks-1) {
            int t=fclose(fw);
            process_file();
        }
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
                    string con_ip=inet_ntoa(address.sin_addr);
                    int con_port=ntohs(address.sin_port);
                    if(!con_ip.compare(trackerIP) && con_port==trackerSendingPort) {
                        cout<<"Tracker connected successfully, IP ADDRESS: "<<con_ip<<", PORT: "<<con_port<<endl;
                        isTrackerUp=true;
                    } else {
                        cout<<"Client connected successfully, IP ADDRESS: "<<con_ip<<", PORT: "<<con_port<<endl;
                    }

                    break;
                }
            }
        }
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds)) {
                if ((valread = read(sd, buffer, buffSize)) <= 0) {
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    string dis_ip=inet_ntoa(address.sin_addr);
                    int dis_port=ntohs(address.sin_port);
                    if(!dis_ip.compare(trackerIP) && dis_port==trackerSendingPort) {
                        isTrackerUp=false;
                        cout<<"Tracker is down, IP ADDRESS: "<<dis_ip<<", PORT: "<<dis_port<<endl;
                    } else
                        cout<<"Client disconnected, IP ADDRESS: "<<inet_ntoa(address.sin_addr)<<", PORT: "<<ntohs(address.sin_port)<<endl;
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    cout<<"Message Recieved!"<<endl;
                    cout.flush();
                    string msg_recieved=buffer;
                    bzero(buffer, buffSize);
                    onMessageRecieved(msg_recieved);
                    msg_recieved.clear();
                    cout<<"Returned from thread"<<endl;
                }
            }
        }
    }
}