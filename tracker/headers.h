#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <fstream>
#include <thread>
#include <sys/stat.h>
#include <fcntl.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;


extern bool isTrackerUp;
extern bool notifyTracker, sendData;
extern json notification;
extern string selfIP, trackerIP, buff_file, tmp_rv_file, sharable_data, seedFilePath;
extern int selfPORT, trackerPORT, selfSendingPort, trackerSendingPort;

extern json seedlist, sharelist;

void listen_for_clients();

void conTracker(string);

void conClient(string, int, string);

bool is_file_exist(char *);