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
#include <thread>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

extern string FTRACKER_IP, STRACKER_IP, SELF_IP, availFilePath;
extern int FTRACKER_PORT, STRACKER_PORT, SELF_PORT, FTRACKER_SENDING_PORT, STRACKER_SENDING_PORT, standard_chunk_size;
extern bool isTracker1Up, isTracker2Up, acquire_lock, asking_seeds, entered_cs;
extern vector <json> download_list, asking_chunks_list;
extern json local_chunks_list;

int create_torrent(string, string);

off_t GetFileLength(string);

void split_string(string, vector<string> &);

void conTracker(string, string, string);

void share_torrent();

void listen_for_connections();

bool is_file_exist(char *);