#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <string.h>
#include <stdexcept>

using namespace std;

extern string FTRACKER_IP, STRACKER_IP, FTRACKER_PORT, STRACKER_PORT;

int create_torrent(string, string);

off_t GetFileLength(string);

void split_string(string, vector<string> &);

void share_torrent();