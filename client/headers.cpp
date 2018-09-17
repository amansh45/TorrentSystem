#include "headers.h"
#include "sha1.h"

string FTRACKER_IP = "10.1.37.60";
string STRACKER_IP = "10.1.37.60";
string FTRACKER_PORT = "7001";
string STRACKER_PORT = "7001";

void share_torrent() {
	
}

void split_string(string command, vector <string>& ptr) {
	//cout<<"helo";
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
    	cout<<hash<<endl;
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
	first_tracker.append(FTRACKER_PORT);
	second_tracker.append(STRACKER_IP);
	second_tracker.push_back(':');
	second_tracker.append(STRACKER_PORT);
	target.close();	
	unsigned long long int file_size = GetFileLength(filepath);
	
	ofstream torrent(torrent_name);
	torrent<<first_tracker<<endl;
	torrent<<second_tracker<<endl;
	torrent<<name<<endl;
	torrent<<file_size<<endl;
	torrent<<final_hash<<endl;
	torrent.close();
	return 0;
}