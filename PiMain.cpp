#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <signal.h>
#include <vector>
#include "libcurl.h"
#include "raspi_config.h"
#include <sys/time.h>

using namespace std;


void int_handler(int x){
    cout << "\nRemoving config from OpenVswitch bridge...";
    cout.flush();
    system("sudo ovs-ofctl del-flows br0");
    system("sudo ovs-ofctl add-flow br0 \"table=0, priority=0, actions=normal\"");
    system("sudo ovs-ofctl add-flow br0 \"table=0, priority=5, actions=drop\"");
    cout << "done\n";
    cout.flush();
    cout << "Removing ./topology.conf ...";
    cout.flush();
    system("sudo rm ./topology.conf");
    cout << "done\n\n";
    cout.flush();
    exit(1);
}


int main(){
    signal(SIGINT,int_handler);
    unsigned int checkWait = 1000 * 1000 * 2.5;//time to wait between checks (in us)
    string controllerURL = "ftp://192.168.254.100/";
    string configFilename = "./topology.conf";
    
    time_t remoteModTime(0);
    time_t localModTime(0);
    struct tm* remoteModtm;
    int status, node;
    ifstream inputFile;
    char tempchar;
    stringstream tempstr;
    
    //determine node number
    inputFile.open("/etc/hostname");
    if (inputFile.fail()){
        cout << "Error opening /etc/hostname.\n";
        return 1;
    }
    inputFile.seekg(-2,ios::end);
    inputFile.get(tempchar);
    tempstr << tempchar;
    tempstr >> node;
    inputFile.close();
    
    cout << "Current node: " << node << "\n";
    
    //Force NTP synchronisation
    system(("sudo service ntp stop && sudo ntpd -gq && sudo service ntp start"));
    
    //Start and initialise OpenVswitch
    status = OVS_init(node);
    
    if (status){
        cout << "Error while initialising OpenVswitch. Exiting now.\n";
        exit(status);
    }
    cout << "Config update monitor now running.\n";
    
    for(;;){
        cout << "\tGetting remote mod time\n";
        //get controller's file modification time
        remoteModTime = curl_get_info(configFilename.c_str(), controllerURL.c_str());
        //if remote newer than local, apply config
        if (remoteModTime > localModTime){
            cout << "\tUpdating local topology config file (" << configFilename << ")\n";
            
            //Wait until trying again if the get failed
            if(curl_get(configFilename.c_str(),controllerURL.c_str())) {
                cout << "\tCould not get remote topology config file.\n";
                usleep(checkWait/2);
                continue;
            }
            
            //Update local modification time
            remoteModtm = localtime(&remoteModTime);
            tempstr.str(string());
            tempstr.clear();
            tempstr << "sudo touch " << configFilename << " -d \"" 
                    << (remoteModtm->tm_year+1900) << "-" << (remoteModtm->tm_mon+1) << "-" << remoteModtm->tm_mday << " "
                    << remoteModtm->tm_hour << ":" << remoteModtm->tm_min << ":" << remoteModtm->tm_sec << "\"";
            system(tempstr.str().c_str());
            localModTime = remoteModTime;

            //Implement topology change
            cout << "\tImplementing topology change\n";
            raspi_config(configFilename, node);
        }
        else {
            //wait for next config check
            cout << "No config change. Sleeping for " << checkWait/1000/1000 << " seconds\n";
            usleep(checkWait);
        }
    }
    return 0;
}