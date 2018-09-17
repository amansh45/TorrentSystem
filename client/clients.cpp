#include "headers.h"
#include "sha1.h"

int main() {
	
	while(true) {
		string command;
		getline(cin, command);
		vector <string> arr;
		split_string(command, arr);
		if(!arr[0].compare("share")) {
			create_torrent(arr[1], arr[2]);
			share_torrent();
		}
	}

    return 0;
}