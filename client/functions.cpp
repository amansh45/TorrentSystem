#include "headers.h"
#include "sha1.h"

string FTRACKER_IP, STRACKER_IP, SELF_IP;
string availFilePath = "seedingfiles.json";
int FTRACKER_PORT, SELF_PORT, STRACKER_PORT;
bool isTracker1Up=false, isTracker2Up=false;
vector <json> download_list;
vector <json> downloaded_chunks_list;
json local_chunks_list;

bool is_file_exist(char *fileName) {
    std::ifstream infile(fileName);
    bool var=infile.good();
    infile.close();
    return var;
}

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
		string file_str, file_name, file_hash;
		int file_size;
		int count=1;
		while(getline(target, file_str)) {
			if(count==3)
				file_name=file_str;
			else if(count==4)
				file_size=stoi(file_str);
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

	    json sample;
	    sample["hash_o_hash"]=hash;
	    sample["local_chunks"]=json::array();
	    int num_chunks=file_size/(512*1024);
	    for(int k=0;k<=num_chunks;k++) {
	    	sample["local_chunks"].push_back(k);
	    }
	    local_chunks_list["local_list"].push_back(sample);

	    ofstream com(availFilePath);
		com<<local_chunks_list;
		com.close();

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
		
		// Remove from other 2 lists also


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

	if(!target.is_open()) {
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

bool check_chunks(int num, vector<int> &chunks_added) {
	for(vector<int>::iterator itr=chunks_added.begin();itr!=chunks_added.end();itr++) {
		int temp=*itr;
		if(temp==num)
			return true;
	}
	return false;
}

void lock_seeders(string hash) {
	cout<<":::::::::::::::"<<endl;
	cout<<download_list<<endl;
	cout<<":::::::::::::::";
	vector <vector <int>> arr;
	vector <string> identity;
	vector <string> new_identity;
	int num=0, alter;
	bool hash_found=false;
	for(vector <json>::iterator itr=download_list.begin();itr!=download_list.end();itr++) {
		cout<<"7"<<endl;
		json json_obj=*itr;
		cout<<"6"<<endl;
		string double_hash=json_obj["hash_o_hash"];
		cout<<"5"<<endl;
		if(!double_hash.compare(hash)) {
			cout<<"4"<<endl;
			hash_found=true;
			json j=json_obj["seeders"];
			cout<<"3"<<endl;
			int t=0;
			num=j.size();
			alter=num;
			arr.resize(num);
			identity.resize(num);
			new_identity.resize(num);
			cout<<j<<endl;
			for(json::iterator ist=j.begin();ist!=j.end();ist++) {
				cout<<"2"<<endl;
				num++;
				json single_seed=*ist;
				json sample;
				sample["ip"]=single_seed["ip"];
				sample["port"]=single_seed["port"];
				string identifier=sample.dump();
				identity.push_back(identifier);
				new_identity.push_back(identifier);

				for(json::iterator it=single_seed["chunks_available"].begin();it!=single_seed["chunks_available"].end();it++) {
					cout<<"adding "<<*it<<" to "<<t<<endl;
					arr[t].push_back(*it);
				}
				t++;
			}
			break;
		}
	}

	vector <vector <int>> new_arr(num);
	vector <vector <int>> final_arr(num);
	vector <vector<int>::iterator> itr_arr(num);
	
	for(int i=0;i<num;i++) {
		itr_arr[i]=arr[i].begin();
	}

	int min_size=distance(arr[0].begin(), arr[0].end());
	int max_size=distance(arr[0].begin(), arr[0].end());
	int del_index=0;

	for(int i=1;i<num;i++) {
		int size=distance(arr[i].begin(), arr[i].end());
		if(size<min_size) {
			min_size=size;
			del_index=i;
		}
		if(size>max_size)
			max_size=size;
	}	
	int prior_index;
	vector <int> chunks_added;
	int rows=0;
	int counter=1;
	while(!arr.empty()) {
		rows+=min_size;
		for(int i=0;i<num;i++) {
			vector<int>::iterator del_itr=itr_arr[del_index];
			if(i==0) {
				prior_index=del_index;
				while(del_itr!=arr[del_index].end()) {
					if(!check_chunks(*del_itr, chunks_added)) {					
						cout<<*del_itr<<" ";
						new_arr[del_index].push_back(*del_itr);
						chunks_added.push_back(*del_itr);
						advance(del_itr, 1);
					} else {
						advance(del_itr, 1);
						itr_arr[del_index]=del_itr;
					}
				}
				cout<<endl;
				if(del_index==0)
					del_index=1;
				else
					del_index=0;
			} else if(del_index!=prior_index) {
				int temp=0;
				while(temp!=min_size) {
					if(!check_chunks(*del_itr, chunks_added)) {
						cout<<*del_itr<<" ";
						new_arr[del_index].push_back(*del_itr);
						chunks_added.push_back(*del_itr);
						advance(del_itr, 1);
						itr_arr[del_index]=del_itr;		
						temp++;
					} else {
						advance(del_itr, 1);
						itr_arr[del_index]=del_itr;
					}
				}
				cout<<endl;
				del_index++;	
			}	
		}

		arr.erase(arr.begin()+prior_index);
		itr_arr.erase(itr_arr.begin()+prior_index);

		string sidentity=identity[prior_index];
		int k;
		for(k=0;k<alter;k++) {
			string samp=new_identity[k];
			if(!samp.compare(sidentity))
				break;
		}
		for(vector<int>::iterator itt=new_arr[prior_index].begin();itt!=new_arr[prior_index].end();itt++) {
			final_arr[k].push_back(*itt);
		}
		new_arr.erase(new_arr.begin()+prior_index);
		identity.erase(identity.begin()+prior_index);
		num-=1;
		min_size=distance(itr_arr[0],arr[0].end());
		del_index=0;
		for(int i=1;i<num;i++) {
			int tsize=distance(itr_arr[i], arr[i].end());
			if(tsize<min_size) {
				min_size=tsize;
				del_index=i;
			}
		}
	}

	for(vector<vector<int>>::iterator itr=final_arr.begin();itr!=final_arr.end();itr++) {
		for(vector<int>::iterator new_itr=itr->begin();new_itr!=itr->end();new_itr++) {
			cout<<*new_itr<<" ";
		}
		cout<<endl;
	}

}


void commClient(string Ip, int Port, string message) {
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

void onMessageRecieved(string data, string type_of_data) {
	if(!type_of_data.compare("json")) {
		json data_obj=json::parse(data);
		string agent=data_obj["agent"];
		string action=data_obj["action"];
		string tip=data_obj["ip"];
		int tport=data_obj["port"];
		if(!agent.compare("tracker")) {
			if(!action.compare("getawk")) {
				string hash=data_obj["data"]["hash_o_hash"];
				for(vector<json>::iterator itr=download_list.begin();itr!=download_list.end();itr++) {
					json download_entry=*itr;
					if(!hash.compare(download_entry["hash_o_hash"])) {
						json seed_arr=data_obj["data"]["seeders"];
						for(json::iterator it=seed_arr.begin();it!=seed_arr.end();it++) {
							json single_seed=*it;
							json wrapper_msg;
							wrapper_msg["ip"]=SELF_IP;
							wrapper_msg["port"]=SELF_PORT;
							wrapper_msg["agent"]="client";
							wrapper_msg["action"]="chunk_details";
							wrapper_msg["hash_o_hash"]=hash;
							string sending_msg=wrapper_msg.dump();
							commClient(single_seed["ip"], single_seed["port"], sending_msg);
							single_seed["chunks_available"]=json::array();
						}
						break;
					}
				}
			}
		} else if(!agent.compare("client")) {
			if(!action.compare("chunk_details")) {
				string hash=data_obj["hash_o_hash"];
				for(json::iterator itr=local_chunks_list["local_list"].begin();itr!=local_chunks_list["local_list"].end();itr++) {
					json entry=*itr;
					if(!hash.compare(entry["hash_o_hash"])) {
						json wrapper_msg;
						wrapper_msg["ip"]=SELF_IP;
						wrapper_msg["port"]=SELF_PORT;
						wrapper_msg["agent"]="client";
						wrapper_msg["action"]="awk_chunk_details";
						wrapper_msg["data"]["hash_o_hash"]=hash;
						wrapper_msg["data"]["local_chunks"]=entry["local_chunks"];
						string sending_msg=wrapper_msg.dump();
						commClient(tip, tport, sending_msg);
					}
				}
			} else if(!action.compare("awk_chunk_details")) {
				vector <json>::iterator itr;
				for(itr=download_list.begin();itr!=download_list.end();itr++) {
					json sample=*itr;
					string hash=sample["hash_o_hash"];
					if(!hash.compare(data_obj["data"]["hash_o_hash"])) {
						json file_seeders=sample["seeders"];
						json seeder;
						seeder["ip"]=tip;
						seeder["port"]=tport;
						seeder["chunks_available"]=json::array();
						for(json::iterator ist=data_obj["data"]["local_chunks"].begin();ist!=data_obj["data"]["local_chunks"].end();ist++) {
							int value=*ist;
							seeder["chunks_available"].push_back(value);
						}
						file_seeders.push_back(seeder);
						sample["seeders"]=file_seeders;
						*itr=sample;
						break;
					}
				}
				lock_seeders(data_obj["data"]["hash_o_hash"]);
			}
		}
	} else if(!type_of_data.compare("file_data")) {
		
	}
	
}

void listen_for_connections() {
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[1000], max_clients=1000, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    int buffSize=1024*512;
    char buffer[buffSize];
    
    fd_set readfds;
      
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
    char self_ip[SELF_IP.length()+1];
    strcpy(self_ip, SELF_IP.c_str());
    self_ip[SELF_IP.length()]=0;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(self_ip);
    address.sin_port = htons(SELF_PORT);
      
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        cout<<"Binding socket to port: "<<SELF_PORT<<" failed";
        return;
    } else
        cout<<"Listening on PORT: "<<SELF_PORT<<endl;


    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
      
    addrlen = sizeof(address);
    cout<<"Waiting for connections!"<<endl;
     
    while(true) {
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
                    string dis_ip=inet_ntoa(address.sin_addr);
                    int dis_port=ntohs(address.sin_port);
                    cout<<"Client disconnected, IP ADDRESS: "<<inet_ntoa(address.sin_addr)<<", PORT: "<<ntohs(address.sin_port)<<endl;
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    cout<<"Message Recieved!"<<endl;
                    cout<<buffer<<endl;
                    cout.flush();
                    string msg_recieved=buffer;
                    bzero(buffer, buffSize);
                    try {
                    	json new_msg=json::parse(msg_recieved);
                    	onMessageRecieved(msg_recieved, "json");
                    } catch(int n) {
                    	cout<<"Message recieved from the client: "<<msg_recieved<<endl;
                    	onMessageRecieved(msg_recieved, "file_data");
                    }
                }
            }
        }
    }
}