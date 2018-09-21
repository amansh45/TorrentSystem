#include "headers.h"
#include "sha1.h"

string FTRACKER_IP, STRACKER_IP, SELF_IP;
int FTRACKER_PORT, SELF_PORT, STRACKER_PORT, FTRACKER_SENDING_PORT, STRACKER_SENDING_PORT;
bool isTracker1Up=false, isTracker2Up=false;
vector <json> download_list;

void conTracker(string action, string torrentPath, string extras) {
	struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr, my_addr1; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        cout<<"Unable to create the socket!"<<endl;
        return; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(FTRACKER_PORT); 


    char tracker_ip[FTRACKER_IP.length()+1];
    strcpy(tracker_ip, FTRACKER_IP.c_str());
    tracker_ip[FTRACKER_IP.length()]=0;
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, tracker_ip, &serv_addr.sin_addr)<=0) { 
        cout<<"Invalid address of another tracker!"<<endl; 
        close(sock);
        return; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        cout<<"Connection with the tracker having ip: "<<FTRACKER_IP<<" port: "<<FTRACKER_PORT<<" unsuccessfull!"<<endl;
        close(sock); 
    } else {
        cout<<"Connection with the tracker having ip: "<<FTRACKER_IP<<" port: "<<FTRACKER_PORT<<" successfull!"<<endl;
        isTracker1Up=true;
    } 
    if(!isTracker1Up) {
    	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
	        cout<<"Unable to create the socket!"<<endl;
	        return; 
	    } 
	   
	    memset(&serv_addr, '0', sizeof(serv_addr)); 
	    
	    serv_addr.sin_family = AF_INET; 
	    serv_addr.sin_port = htons(STRACKER_PORT); 

	    bzero(tracker_ip, FTRACKER_IP.length()+1);
	    strcpy(tracker_ip, STRACKER_IP.c_str());
	    tracker_ip[STRACKER_IP.length()]=0;
	       
	    // Convert IPv4 and IPv6 addresses from text to binary form 
	    if(inet_pton(AF_INET, tracker_ip, &serv_addr.sin_addr)<=0) { 
	        cout<<"Invalid address of another tracker!"<<endl; 
	        close(sock);
	        return; 
	    }
	    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
	        cout<<"Connection with the tracker having ip: "<<STRACKER_IP<<" port: "<<STRACKER_PORT<<" unsuccessfull!"<<endl;
	        close(sock);
	        cout<<"Both the trackers are down!";
	        return; 
	    } else {
	        cout<<"Connection with the tracker having ip: "<<STRACKER_IP<<" port: "<<STRACKER_PORT<<" successfull!"<<endl;
	        isTracker2Up=true;
	    }

    }
    if(!action.compare("share")) {
    	ifstream target(torrentPath);
		string file_str, file_name, file_hash, file_size;
		int count=1;
		while(getline(target, file_str)) {
			if(count==3)
				file_name=file_str;
			else if(count==4)
				file_size=file_str;
			else if(count==5)
				file_hash=file_str;
			file_str.clear();
			count++;
		}
		SHA1 checksum;
		checksum.update(file_hash);
	    string hash = checksum.final();
	    json j;
	    j["agent"]="client";
	    j["action"]="share";
	    j["ip"]=SELF_IP;
	    j["port"]=SELF_PORT;
	    j["data"]["hash"]=hash;
	    j["data"]["filename"]=file_name;
	    j["data"]["client"]["ip"]=SELF_IP;
	    j["data"]["client"]["port"]=SELF_PORT;
	    j["data"]["seeders"]=json::array();
	    j["data"]["seeders"][0]["ip"]=SELF_IP;
	    j["data"]["seeders"][0]["port"]=SELF_PORT;

	    string sharingData=j.dump();
	    char shareData[sharingData.length()+1];
	    strcpy(shareData, sharingData.c_str());
	    shareData[sharingData.length()]=0;
	    send(sock, shareData, strlen(shareData), 0);
	    close(sock);
	    target.close();
    } else if(!action.compare("get")) {
    	ifstream target(torrentPath);
		string file_str, hash_o_hash, file_name, file_hash, file_size;
		int count=1;
		while(getline(target, file_str)) {
			if(count==3)
				file_name=file_str;
			else if(count==4)
				file_size=file_str;
			else if(count==5)
				file_hash=file_str;
			file_str.clear();
			count++;
		}
		SHA1 checksum;
		checksum.update(file_hash);
		hash_o_hash=checksum.final();
		json j;
		j["file_size"]=file_size;
		j["file_hash"]=file_hash;
		j["file_name"]=file_name;
		j["down_path"]=extras;
		j["seeders"]=json::array();
		j["hash_o_hash"]=hash_o_hash;
		download_list.push_back(j);

		json toTrackerData;
		toTrackerData["ip"]=SELF_IP;
		toTrackerData["port"]=SELF_PORT;
		toTrackerData["agent"]="client";
		toTrackerData["action"]="get";
		toTrackerData["hash_o_hash"]=hash_o_hash;
		cout<<toTrackerData<<endl;
		string send_data=toTrackerData.dump();
		
		char char_sdata[send_data.length()+1];
		strcpy(char_sdata, send_data.c_str());
		char_sdata[send_data.length()]=0;
		send(sock, char_sdata, strlen(char_sdata), 0);
	    close(sock);
	    target.close();
    } else if(!action.compare("remove")) {
    	ifstream target(torrentPath);
		string file_str, hash_o_hash, file_hash;
		int count=1;
		while(getline(target, file_str)) {
			if(count==5)
				file_hash=file_str;
			file_str.clear();
			count++;
		}
		SHA1 checksum;
		checksum.update(file_hash);
		hash_o_hash=checksum.final();
		json j;
		vector<json>::iterator itr;
		bool hash_found=false;
		for(itr=download_list.begin();itr!=download_list.end();itr++) {
			json json_obj=*itr;
			string double_hash=json_obj["hash_o_hash"];
			if(double_hash.compare(hash_o_hash)) {
				hash_found=true;
				break;
			}
		}
		if(hash_found)
			download_list.erase(itr);
		
		json toTrackerData;
		toTrackerData["agent"]="client";
		toTrackerData["action"]="remove";
		toTrackerData["ip"]=SELF_IP;
		toTrackerData["port"]=SELF_PORT;
		toTrackerData["hash_o_hash"]=hash_o_hash;
		string send_data=toTrackerData.dump();
		char char_sdata[send_data.length()+1];
		strcpy(char_sdata, send_data.c_str());
		char_sdata[send_data.length()]=0;
		send(sock, char_sdata, strlen(char_sdata), 0);
	    close(sock);
	    target.close();
	    char ctorrPath[torrentPath.length()+1];
	    strcpy(ctorrPath, torrentPath.c_str());
	    ctorrPath[torrentPath.length()]=0;
	    if( remove(ctorrPath) != 0 )
		    cout<<"Error in removing mTorrent File: "<<torrentPath<<endl;
		else
		    cout<<"mTorrent File: "<<torrentPath<<" removed successfully!"<<endl;
	} else if(!action.compare("addSeed")) {
		json toTrackerData;
		toTrackerData["agent"]="client";
		toTrackerData["action"]="addSeed";
		toTrackerData["ip"]=SELF_IP;
		toTrackerData["port"]=SELF_PORT;
		toTrackerData["hash_o_hash"]=extras;
		string send_data=toTrackerData.dump();
		char char_sdata[send_data.length()+1];
		strcpy(char_sdata, send_data.c_str());
		char_sdata[send_data.length()]=0;
		send(sock, char_sdata, strlen(char_sdata), 0);
	    close(sock);
	}
}

void split_string(string command, vector <string>& ptr) {
	string sample;
	int k=0;
	for(int i=0;i<=command.length();i++) {
		if(command[i]!=' ' && command[i]!=0)
			sample.push_back(command[i]);
		else {
			string new_sample;
			new_sample.assign(sample);
			ptr.push_back(new_sample);
			sample.clear();
		}
	}
}

off_t GetFileLength(string filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st) == -1)
        throw runtime_error(strerror(errno));
    return st.st_size;
}

int create_torrent(string filepath, string torrent_name) {
	SHA1 checksum;
	ifstream target(filepath, ifstream::in | ifstream::binary);

	int bufferSize = 1024*512;
	char buffer[bufferSize];
	string final_hash;

	if(!target.is_open())
    {
        return -1;
    }

	do {
	    target.read(buffer, bufferSize);
	    checksum.update(buffer);
    	string hash = checksum.final();
    	string str_hash = hash.substr(0,20);
    	final_hash.append(str_hash);
	} while(target);

	string name;
	int size=filepath.length();
	
	for(int i=size-1;i>=0;i--) {
		if(filepath[i]=='/')
			break;
		else
			name.push_back(filepath[i]);
	}
	reverse(name.begin(), name.end());
	string first_tracker, second_tracker;
	first_tracker.append(FTRACKER_IP);
	first_tracker.push_back(':');
	first_tracker.append(to_string(FTRACKER_PORT));
	second_tracker.append(STRACKER_IP);
	second_tracker.push_back(':');
	second_tracker.append(to_string(STRACKER_PORT));
	target.close();	
	unsigned long long int file_size = GetFileLength(filepath);
	
	ofstream torrent(torrent_name);
	torrent<<first_tracker<<endl;
	torrent<<second_tracker<<endl;
	torrent<<name<<endl;
	torrent<<file_size<<endl;
	torrent<<final_hash<<endl;
	torrent.close();
	conTracker("share", torrent_name, "");
	return 0;
}

// void listen_for_clients() {
//     int opt = 1;
//     int master_socket, addrlen, new_socket, client_socket[1000], max_clients=1000, activity, i, valread, sd;
//     int max_sd;
//     struct sockaddr_in address;
//     int buffSize=1024*62;
//     char buffer[buffSize];
      
//     fd_set readfds;
      
//     char *message = "ECHO Daemon v1.0 \r\n";
    
//     for (i = 0; i < max_clients; i++) 
//         client_socket[i] = 0;
    
//     if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
//         cout<<"Unable to create the master socket!"<<endl;
//     else
//         cout<<"Creation of master socket is successfull!"<<endl;
    
//     // int flags=fcntl(master_socket, F_GETFL);
//     // fcntl(master_socket, F_SETFL, flags | O_NONBLOCK);
//     //cout<<SetSocketBlockingEnabled(master_socket, true);

//     if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
//         cout<<"Unable to allow master socket for multiple connections!"<<endl;
//         return;
//     }
//     char self_ip[selfIP.length()+1];
//     strcpy(self_ip, selfIP.c_str());
//     self_ip[selfIP.length()]=0;
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = inet_addr(self_ip);
//     address.sin_port = htons(selfPORT);
      
//     if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
//         cout<<"Binding socket to port: "<<selfPORT<<" failed";
//         return;
//     } else
//         cout<<"Listening on PORT: "<<selfPORT<<endl;


//     if (listen(master_socket, 3) < 0) {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
      
//     addrlen = sizeof(address);
//     cout<<"Waiting for connections!"<<endl;
     
//     while(true) 
//     {
//         FD_ZERO(&readfds);
//         FD_SET(master_socket, &readfds);
//         max_sd = master_socket;
        
//         for(i=0;i<max_clients;i++) {
//             sd = client_socket[i];
//             if(sd > 0)
//                 FD_SET(sd, &readfds);
//             if(sd > max_sd)
//                 max_sd = sd;
//         }
//         activity = select(max_sd+1, &readfds, NULL, NULL, NULL);
          
//         //If something happened on the master socket , then its an incoming connection
//         if(FD_ISSET(master_socket, &readfds)) {
//             if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
//                 perror("accept");
//                 exit(EXIT_FAILURE);
//             }
//             for (i=0;i<max_clients;i++) {
//                 if( client_socket[i] == 0 ) {
//                     client_socket[i] = new_socket;
//                     cout<<"Client connected successfully, IP ADDRESS: "<<inet_ntoa(address.sin_addr)<<", PORT: "<<ntohs(address.sin_port)<<endl;
//                     break;
//                 }
//             }
//         }
//         for (i = 0; i < max_clients; i++) {
//             sd = client_socket[i];
//             if (FD_ISSET( sd , &readfds)) {
//                 //Check if it was for closing , and also read the incoming message
//                 if ((valread = read(sd, buffer, buffSize)) <= 0) {
//                     getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
//                     string dis_ip=inet_ntoa(address.sin_addr);
//                     int dis_port=ntohs(address.sin_port);
//                     if(!dis_ip.compare(trackerIP) && dis_port==trackerSendingPort) {
//                         isTrackerUp=false;
//                         cout<<"Tracker is down, IP ADDRESS: "<<dis_ip<<", PORT: "<<dis_port<<endl;
//                     } else
//                         cout<<"Client disconnected, IP ADDRESS: "<<inet_ntoa(address.sin_addr)<<", PORT: "<<ntohs(address.sin_port)<<endl;
//                     close(sd);
//                     client_socket[i] = 0;
//                 } else {
//                     cout<<"Message Recieved!"<<endl;
//                     cout.flush();
//                     string msg_recieved=buffer;
//                     bzero(buffer, buffSize);
//                     onMessageRecieved(msg_recieved);
//                     msg_recieved.clear();
//                     cout<<"Returned from thread";
//                 }
//             }
//         }
//     }
// }