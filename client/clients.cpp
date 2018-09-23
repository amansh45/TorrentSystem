#include "headers.h"
#include "sha1.h"

int main(int argc , char *argv[]) {
	string self=argv[1];
	string first_tracker=argv[2];
    string second_tracker=argv[3];
    bool ip=false, port=false;
    string ftracker_ip, stracker_ip, self_ip;
    int stracker_port=0, ftracker_port=0, self_port=0;
    for(int i=0;i<first_tracker.length();i++) {
        if(first_tracker[i]==':')
            ip=true;
        else if(ip==false)
            ftracker_ip.push_back(first_tracker[i]);
        else if(port==false) 
            ftracker_port=(ftracker_port*10)+(first_tracker[i]-'0');
    }
    ip=false;
    port=false;

    for(int i=0;i<second_tracker.length();i++) {
        if(second_tracker[i]==':')
            ip=true;
        else if(ip==false)
            stracker_ip.push_back(second_tracker[i]);
        else if(port==false)
            stracker_port=(stracker_port*10)+(second_tracker[i]-'0');
    }
    ip=false;
    port=false;
    for(int i=0;i<self.length();i++) {
        if(self[i]==':')
            ip=true;
        else if(ip==false)
            self_ip.push_back(self[i]);
        else if(port==false)
            self_port=(self_port*10)+(self[i]-'0');
    }

    STRACKER_IP=stracker_ip;
    STRACKER_PORT=stracker_port;
    FTRACKER_IP=ftracker_ip;
    FTRACKER_PORT=ftracker_port;
    SELF_IP=self_ip;
    SELF_PORT=self_port;

    auto init = []() { 
        while(true) {
            string command;
            getline(cin, command);
            vector <string> arr;
            split_string(command, arr);
            if(!arr[0].compare("share")) {
                create_torrent(arr[1], arr[2]);
            } else if(!arr[0].compare("get")) {
                conTracker("get", arr[1], arr[2]);
            } else if(!arr[0].compare("remove")) {
                conTracker("remove", arr[1], "");
            }
        } 
    };

    char p[availFilePath.length()+1];
    strcpy(p, availFilePath.c_str());
    p[availFilePath.length()]=0;

    if(is_file_exist(p)) {
        ifstream downloadedlist(availFilePath);
        downloadedlist>>local_chunks_list;
    }

    thread th1(init);
    thread th2(listen_for_connections);
    th1.join();
    th2.join();

    return 0;
}