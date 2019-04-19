#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>
#include <fstream>
#include <bitset>
#include <cstring>
#include "topology.h"
using namespace std;


typedef unsigned long long timestamp_t;
static timestamp_t get_timestamp (){
  struct timeval now;
  gettimeofday (&now, NULL);
  return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}


int main(int argc, const char * argv[]) {
    int MEASURE_INTERVAL = 15;//interval in seconds between traffic calculations

    int NUM_NODES = 8;//number of nodes in network
    //node numbers represented as their matrix indices
    int N1 = 0; int N2 = 1; int N3 = 2; int N4 = 3;
    int N5 = 4; int N6 = 5; int N7 = 6; int N8 = 7;
    
    fstream configNew, configOld;
    string configFilename = "./topology.conf";

    int i,j, agent, source, dest, packetSize, sampleRate;
    vector<int> trafficCountCurrent(NUM_NODES*NUM_NODES);//traffic count for each S-D pair (this interval)
    vector<int> trafficCountLast(NUM_NODES*NUM_NODES);//traffic count for each S-D pair (previous interval)
    vector<double> traffic(NUM_NODES*NUM_NODES);//traffic for each S-D pair
    double tdiff, totalTraffic;
    double tdifflast = 0;
    string input, dumpstr, lineOld, lineNew, line;
    char tempchar;
    stringstream tempstr;
    adjacency_list_t adjacency_list(NUM_NODES);
    timestamp_t t0;//used to calculate measurement interval
    bool topologyChange;

    //initialise adjacency matrix (adjacent node, link weight)
    adjacency_list[N1].push_back(neighbor(N2, 10));
    adjacency_list[N1].push_back(neighbor(N6, 10));
    adjacency_list[N1].push_back(neighbor(N7, 1));

    adjacency_list[N2].push_back(neighbor(N1, 10));
    adjacency_list[N2].push_back(neighbor(N3, 10));
    adjacency_list[N2].push_back(neighbor(N7, 10));

    adjacency_list[N3].push_back(neighbor(N2, 10));
    adjacency_list[N3].push_back(neighbor(N4, 10));
    adjacency_list[N3].push_back(neighbor(N5, 1));
    adjacency_list[N3].push_back(neighbor(N8, 1));

    adjacency_list[N4].push_back(neighbor(N3, 10));
    adjacency_list[N4].push_back(neighbor(N8, 10));

    adjacency_list[N5].push_back(neighbor(N3, 1));
    adjacency_list[N5].push_back(neighbor(N6, 10));

    adjacency_list[N6].push_back(neighbor(N1, 10));
    adjacency_list[N6].push_back(neighbor(N5, 10));
    adjacency_list[N6].push_back(neighbor(N7, 10));

    adjacency_list[N7].push_back(neighbor(N1, 1));
    adjacency_list[N7].push_back(neighbor(N2, 10));
    adjacency_list[N7].push_back(neighbor(N6, 10));
    adjacency_list[N7].push_back(neighbor(N8, 10));

    adjacency_list[N8].push_back(neighbor(N3, 1));
    adjacency_list[N8].push_back(neighbor(N4, 10));
    adjacency_list[N8].push_back(neighbor(N7, 10));
    
    //initialise traffic counts
    for (i=0;i<trafficCountCurrent.size();i++){
        trafficCountCurrent[i] = 0;
        trafficCountLast[i] = 0;
    }
    //capture start time
    t0 = get_timestamp();
    while (1) {
        //clear data from last capture
        totalTraffic = 0;
        tempstr.str(string());
        tempstr.clear();

        //capture traffic data
        getline(cin,line);
        tempstr << line;
        getline(tempstr,line,',');
        //skip processing if sFlow message is not a flow sample
        if(strcmp(line.c_str(),("FLOW"))==0){
            //get agent node number
            getline(tempstr,dumpstr,'.');//dump first octet of agent address
            getline(tempstr,dumpstr,'.');//dump second octet of agent address
            getline(tempstr,line,'.');//capture third octet of agent address
            agent = atof(line.c_str());
            if (agent%10 != 0){//discard message if the agent is an unexpected value
                continue;
            }
            agent /= 10;
            
            //Unwanted data
            getline(tempstr,dumpstr,',');//dump remainder of agent address
            getline(tempstr,dumpstr,',');//dump input port
            getline(tempstr,dumpstr,',');//dump output port
            getline(tempstr,dumpstr,',');//dump source mac
            getline(tempstr,dumpstr,',');//dump dest mac
            getline(tempstr,dumpstr,',');//dump ethertype
            getline(tempstr,dumpstr,',');//dump in vlan
            getline(tempstr,dumpstr,',');//dump out vlan
            
            //get source node number
            tempstr.get(tempchar);
            if (tempchar=='-'){//discard message if the source IP is not specified
                continue;
            }
            getline(tempstr,dumpstr,'.');//dump first octet of source address
            getline(tempstr,dumpstr,'.');//dump second octet of sourse address
            getline(tempstr,line,'.');//capture third octet of source address
            source = atof(line.c_str());
            if (source%10 != 0){//discard message if the source is an unexpected value
                continue;
            }
            source /= 10;
            getline(tempstr,dumpstr,',');//dump remainder of source address
            
            //get destination node number
            getline(tempstr,dumpstr,'.');//dump first octet of dest address
            getline(tempstr,dumpstr,'.');//dump second octet of destaddress
            getline(tempstr,line,'.');//capture third octet of dest address
            dest = atof(line.c_str());
            if (dest%10 != 0){
            //    cout << "Halting message processing\n\n";
                continue;
            }
            dest /= 10;
            getline(tempstr,dumpstr,',');//dump remainder of dest address
            
            //unwanted data
            getline(tempstr,dumpstr,',');//dump IP protocol
            getline(tempstr,dumpstr,',');//dump IP TOS
            getline(tempstr,dumpstr,',');//dump IP TTL
            getline(tempstr,dumpstr,',');//dump TCP/UDP source port
            getline(tempstr,dumpstr,',');//dump TCP/UDP dest port
            getline(tempstr,dumpstr,',');//dump TCP flags
            
            //unwanted data
            getline(tempstr,line,',');//dump packet size
            
            //get IP packet size
            getline(tempstr,dumpstr,',');//capture IP packet size
            packetSize = atof(line.c_str());
            
            //get sampling rate
            getline(tempstr,line);//capture sampling rate
            sampleRate = atof(line.c_str());
            
            //increment relevant S/D traffic counter
            if (agent==dest) trafficCountCurrent[(source-1)*NUM_NODES+(dest-1)] += (packetSize*sampleRate);

            //find time since last calculation
            tdiff = (double)((get_timestamp() - t0)/1000000.0L);
            
            //Calculate traffic rates if the measurement interval has passed
            if (tdiff >= MEASURE_INTERVAL){
                //capture new start time
                t0 = get_timestamp();
                
                //calculate traffic demands using samples from last two intervals
                for (i=0;i<trafficCountCurrent.size();i++){
                    traffic[i] = (double)((trafficCountCurrent[i] + trafficCountLast[i]) * 8 / (tdiff+tdifflast) / 1000000.0L);
                }
                
                //store time difference
                tdifflast = tdiff;
                
                //store samples from last interval
                trafficCountLast = trafficCountCurrent;
                
                //reinitialise traffic counters and show current traffic
	        cout << "= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
                cout << "Traffic (Mbps): \n" << setprecision(2) << left;
                for (i=0;i<trafficCountCurrent.size();i++){
                    trafficCountCurrent[i] = 0;
                    cout << "    " << (int)(i/NUM_NODES)+1 << "-" << (i%NUM_NODES)+1 << ": " << setw(6) << traffic[i] << " Mbps";
                    if (i%NUM_NODES==7)
                        cout << "\n";
                    totalTraffic = totalTraffic + traffic[i];
                }
                cout << "Total Traffic (Mbps): " << totalTraffic;
	        cout << "\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";

                //create new topology file
                system(("sudo touch " + configFilename + ".new").c_str());
                system(("sudo chown daniel " + configFilename + ".new").c_str());
                system(("sudo chgrp daniel " + configFilename + ".new").c_str());
                
                
                //write config to new topology file
                //If argc > 1, the dynamic topology mechanism will be disabled
                topology_select(traffic, adjacency_list, configFilename + ".new", argc);
                
                //Open config files
                configNew.open((configFilename + ".new").c_str());
                if (configNew.fail()){
                    cout << "Error opening " << configFilename << ".new\n";
                    return 1;
                }
                configOld.open(configFilename.c_str());
                if (configOld.fail()){
                    //In case the file doesn't exist, try to create it and try again
                    system(("sudo touch " + configFilename).c_str());
                    configOld.open(configFilename.c_str());
                    if (configOld.fail()){
                        cout << "Error opening " << configFilename << "\n";
                        return 1;
                    }
                }
                
                //check if the current topology matches the new topology (prevents unneccessary processing by the nodes)
                topologyChange = false;
                while(!configOld.eof() && !configNew.eof()){
                    getline(configOld,lineOld);
                    getline(configNew,lineNew);
                    if (strcmp(lineOld.c_str(),string().c_str())==0 || strcmp(lineOld.c_str(),lineNew.c_str())!=0){
                        topologyChange = true;
                        break;
                    }
                }
                configOld.close();
                configNew.close();
                if (topologyChange){
                    //overwrite old config file
                    cout << "Updating topology\n";
                    system(("sudo mv " + configFilename + ".new " + configFilename).c_str());
                }
                else{
                    cout << "No topology change\n";
                    system(("sudo rm " + configFilename + ".new").c_str());
                }
                
	        cout << "= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
            }
        }
    }
    return 0;
}