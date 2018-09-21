#include "headers.h"

int main(int argc , char *argv[])
{
    string first_tracker=argv[1];
    string second_tracker=argv[2];
    bool ip=false, port=false;
    string ftracker_ip, stracker_ip;
    int stracker_port=0, ftracker_port=0;
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
    
    seedFilePath = "seederlist.json";

    ifstream seederlist(seedFilePath);
    seederlist>>seedlist;
    seederlist.close();

    char transfile[buff_file.length()+1];
    strcpy(transfile, buff_file.c_str());
    transfile[buff_file.length()]=0;

    if(is_file_exist(transfile)) {
        ifstream transseederlist(buff_file);
        transseederlist>>sharelist;
        transseederlist.close();    
    }
    
    selfIP=ftracker_ip;
    selfPORT=ftracker_port;
    trackerIP=stracker_ip;
    trackerPORT=stracker_port;
    selfSendingPort=selfPORT+33;
    trackerSendingPort=trackerPORT+33;
    
    thread th1(conTracker, "onBootSend");
    
    thread th2(listen_for_clients);
    
    th1.join();
    
    th2.join();
    return 0;
}