#ifndef RASPI_CONFIG_H
#define RASPI_CONFIG_H
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <ctime>
#include <sys/time.h>

using namespace std;

int raspi_config(string configFilename, int node);
string get_lxc_mac(int node);
int OVS_start();
int OVS_init(int node);
int LXC_start(int node);
#endif