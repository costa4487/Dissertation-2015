#include "raspi_config.h"
using namespace std;
 
int raspi_config(string configFilename, int node){    
    int NUM_NODES = 8;
    short activeNodes;
    int i, j, port;
    int status = 0;
    stringstream tempstr;
    ifstream inputFile;
    string line;
    vector<int> nextHops(NUM_NODES);
    vector<int> interfaces(4);
    vector<int> outputPorts(NUM_NODES);
    
    struct ifaddrs* ifAddrStruct = NULL;
    struct ifaddrs* ifa = NULL;
    void * tmpAddrPtr = NULL;
    
    bool flag;
    timeval t0, t1;
    struct tm* now;
    struct tm* change;
    
    //synchronise topology change using system clock
    //get local time
    gettimeofday(&t0, NULL);
    now = localtime(&t0.tv_sec);
    change = localtime(&t0.tv_sec);
    //Output current time
    cout << "\t\tCurrent time: ";
    if (now->tm_hour < 10) cout << "0";
    cout << now->tm_hour << ":";
    if (now->tm_min < 10) cout << "0";
    cout << now->tm_min << ":";
    if (now->tm_sec < 10) cout << "0";
    cout << now->tm_sec << ":";
    if (t0.tv_usec/1000 < 1000) cout << "0";
    if (t0.tv_usec/1000 < 100) cout << "0";
    if (t0.tv_usec/1000 < 10) cout << "0";
    cout << t0.tv_usec/1000 << "\n";

    //determine test start time - closest 5s interval 
    if (change->tm_sec < 5)
        change->tm_sec = 10;
    else if (change->tm_sec >= 5 && change->tm_sec < 10)
        change->tm_sec = 15;
    else if (change->tm_sec >= 10 && change->tm_sec < 15)
        change->tm_sec = 20;
    else if (change->tm_sec >= 15 && change->tm_sec < 20)
        change->tm_sec = 25;
    else if (change->tm_sec >= 20 && change->tm_sec < 25)
        change->tm_sec = 30;
    else if (change->tm_sec >= 25 && change->tm_sec < 30)
        change->tm_sec = 35;
    else if (change->tm_sec >= 30 && change->tm_sec < 35)
        change->tm_sec = 40;
    else if (change->tm_sec >= 35 && change->tm_sec < 40)
        change->tm_sec = 45;
    else if (change->tm_sec >= 40 && change->tm_sec < 45)
        change->tm_sec = 50;
    else if (change->tm_sec >= 45 && change->tm_sec < 50)
        change->tm_sec = 55;
    else if (change->tm_sec >= 50 && change->tm_sec < 55){
        change->tm_sec = 0;
        change->tm_min++;}
    else if (change->tm_sec >= 55){
        change->tm_sec = 5;
        change->tm_min++;}
    
    if (change->tm_min >= 60){ 
        change->tm_hour++;
        change->tm_min -= 60;
    }
    if (change->tm_hour >= 24)
        change->tm_hour -= 24;
    t1.tv_sec = mktime(change);
    t1.tv_usec = 0;
    
    //Output change time
    cout << "\t\tChange time: ";
    if (change->tm_hour < 10) cout << "0";
    cout << change->tm_hour << ":";
    if (change->tm_min < 10) cout << "0";
    cout << change->tm_min << ":";
    if (change->tm_sec < 10) cout << "0";
    cout << change->tm_sec << ":";
    if (t1.tv_usec/1000 < 1000) cout << "0";
    if (t1.tv_usec/1000 < 100) cout << "0";
    if (t1.tv_usec/1000 < 10) cout << "0";
    cout << t1.tv_usec/1000 << "\n";

    //Open config file
    inputFile.open(configFilename.c_str());
    if (inputFile.fail()){
        cout << "Error opening " << configFilename << "\n";
        return 1;;
    }
    //discard lines of the config file until the current node's entry is reached
    for (i=0;i<node-1;i++) getline(inputFile,line);
    //get next hop for each destination
    for (i=0;i<NUM_NODES;i++){
        getline(inputFile,line,',');
        nextHops[i] = atof(line.c_str());
    }
    inputFile.close();
    
    //Get IP addresses of each interface
    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        // check it is IP4 and an ethernet port
        if (ifa->ifa_addr->sa_family == AF_INET && ifa->ifa_name[0] == 'e') { 
            tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (addressBuffer[8] == addressBuffer[11])
                interfaces[atoi(&ifa->ifa_name[3])] = atoi(&addressBuffer[8])%10;
            if (addressBuffer[9] == addressBuffer[11])
                interfaces[atoi(&ifa->ifa_name[3])] = atoi(&addressBuffer[8])/10;
        }
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    
    //convert next hop node number to interface number using IP of interfaces
    for (i=0;i<nextHops.size();i++){
        if (nextHops[i]==node){
            outputPorts[i] = 1;
            continue;
        }
        for (j=0;j<interfaces.size();j++){
            if (interfaces[j]==nextHops[i]) outputPorts[i] = j+2;
        }
    }
    
    //wait until local time == change time
    cout << "\t\tWaiting for topology change time\n\t\t" << ((t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000) << " seconds until start";
    while((t1.tv_sec - t0.tv_sec)*1000 + (t1.tv_usec - t0.tv_usec)/1000 > 0){
        gettimeofday(&t0, NULL);
        if (int((t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000) % 30 == 0){
            if (flag){
                cout << "\n\t\t" << (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000 << " seconds until start";
                flush(cout);
                flag = 0;
            }
        }
        else if (int((t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000) % 3 == 0){
            if (flag){
                cout << ".";
                flush(cout);
                flag = 0;
            }
        }
        else flag =1;
    }
    cout << "\n";
    
    //use next hop OVS port numbers to update OVS flows
    for (i=0;i<NUM_NODES;i++){
        if(i!=node-1){
            tempstr.str(string());//clear string stream for use
            tempstr.clear();
            tempstr << "sudo ovs-ofctl mod-flows br0 \"table=" << i+1 << ", actions=dec_mpls_ttl," << outputPorts[i] << "\"";
            status = system(tempstr.str().c_str());
        }
    }
    if (status){
        cout << "Error modifying transit traffic's flows\n";
        exit(status);
    }
    
    return 0;
}
        
        
        
        

int OVS_init(int node){
    int status = OVS_start();//Start OpenVswitch
    if (status){
        cout << "Error while starting OpenVswitch. Aborting initialisation.\n";
        exit(status);
    }
    int i,j;
    char tempchar;
    stringstream tempstr;
    string lxcmac, intmac;
    ifstream inputFile;
    string intFilename = "/sys/class/net/veth0/address";
    
    //Start the host (runs in an LXC container)
    LXC_start(node);
    
    //get LXC conatiner's MAC address
    lxcmac = get_lxc_mac(node);
    
    //get interface to LXC container's MAC address
    inputFile.open(intFilename.c_str());
    if (inputFile.fail()){
        cout << "Error opening " << intFilename << ".\n";
        return 1;
    }
    getline(inputFile,intmac);
    
    
    cout << "Initialising OpenVswitch configuration:\n\tOutput handling flows...";
    cout.flush();
    //Output handling tables - Initialised as normal operation, modified later using config file
    for (i=1;i<=8;i++){
        tempstr.str(string());//clear string stream for use
        tempstr.clear();
        tempstr << "sudo ovs-ofctl add-flow br0 \"table=" << i << "actions=normal\"";
        status = system(tempstr.str().c_str());
        if (status){
            cout << "Error adding output tables\n";
            return status;
        }
    }
    cout << "done.\n\tTransit and terminating traffic flows...";
    cout.flush();
    //Transit and terminating traffic
    for (i=1;i<=8;i++){
        for (j=1;j<=8;j++){
            if (i != j){
                tempstr.str(string());//clear string stream for use
                tempstr.clear();
                tempstr << "sudo ovs-ofctl add-flow br0 \"table=0, priority=500, dl_type=0x8847, mpls_label=" << j << "0" << i << ", actions=goto_table:" << i << "\"";
                status = system(tempstr.str().c_str());
                if (status){
                    cout << "Error adding transit/terminating flows\n";
                    return status;
                }
            }
        }
    }
    //Terminating traffic output flow (won't change when topology changes)
    //In a real network, this wouldn't work, as it would direct all traffic to a single host (same MAC).
    tempstr.str(string());//clear string stream for use
    tempstr.clear();
    tempstr << "sudo ovs-ofctl mod-flows br0 \"table=" << node << ", actions=pop_mpls:0x0800,mod_dl_src:" << intmac << ",mod_dl_dst:" << lxcmac << ",1\"";
    status = system(tempstr.str().c_str());
    if (status){
        cout << "Error modifying terminating traffic processing flow\n";
        exit(status);
    }
    
    
    cout << "done.\n\tOriginating traffic flows...";
    cout.flush();
    //Originating traffic
    for (i=1;i<=8;i++){
        if (i != node){
            tempstr.str(string());//clear string stream for use
            tempstr.clear();
            tempstr << "sudo ovs-ofctl add-flow br0 \"table=0, priority=1000, in_port=1, dl_type=0x0800, nw_dst=192.168." << i << "0.0/24, actions=push_mpls:0x8847, set_mpls_label:" << node << "0" << i << ", goto_table:" << i <<"\"";
            status = system(tempstr.str().c_str());
            if (status){
                cout << "Error adding originating flows\n";
                return status;
            }
        }
    }
    //Firewall to prevent double-handling by both OVS and Quagga
    status = system("sudo ovs-ofctl add-flow br0 \"table=0, priority=5, actions=drop\"");
    if (status){
        cout << "Error adding firewall flow\n";
        return status;
    }
    cout << "done.\n";
    cout.flush();
    
    return 0;
}


int OVS_start(){
    //Check if OVS is running, start it if required
    cout << "Checking if OpenVswitch is running...";
    cout.flush();
    struct stat buffer;
    int status = 0;
    string ovsdbFilepath = "/usr/local/var/run/openvswitch/ovsdb-server.pid";
    string vswitchdFilepath = "/usr/local/var/run/openvswitch/ovs-vswtichd.pid";
    //check if ovsdb-server.pid AND ovs-vswtichd.pid exist
    if(stat(ovsdbFilepath.c_str(), &buffer) && stat(vswitchdFilepath.c_str(), &buffer)){
        cout << "no.\nStarting OpenVswitch...";
        cout.flush();
        status = system("sudo ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock --remote=db:Open_vSwitch,Open_vSwitch,manager_options --pidfile --detach");
        status = status + system("sudo ovs-vsctl --no-wait init");
        status = status + system("sudo ovs-vswitchd --pidfile --detach");
        if (status){
            cout << "\nError starting OpenVswitch\n";
            return status;
        }
        cout << "done.\n";
    }
    else
        cout << "yes.\n";
    return 0;
}

int LXC_start(int node){
    stringstream tempstr;
    int status = 0;
    
    cout << "Checking if LXC container \"host" << node << "\" is running...";
    
    tempstr.str(string());//clear string stream for use
    tempstr.clear();
    tempstr << "sudo LD_LIBRARY_PATH=/usr/local/lib lxc-info -n host" << node << " | grep STOPPED";
    
    if (!system(tempstr.str().c_str())){//system returns '0' if host is stopped
        cout << "no.\nStarting LXC container \"host" << node << "\"...";
    
        tempstr.str(string());//clear string stream for use
        tempstr.clear();
        tempstr << "sudo LD_LIBRARY_PATH=/usr/local/lib lxc-start -n host" << node;
        status = system(tempstr.str().c_str());
        
        if (status){
            cout << "failed :(\n";
            return status;
        }
        
        cout << "done.\n";
    }
    else
        cout << "yes.\n";
    return 0;
}

string get_lxc_mac(int node){
    ifstream inputFile;
    string arpFilename = "/proc/net/arp";
    stringstream tempstr;
    string line, dump, lxcmac;
    vector<string> ipaddr(4);
    int i;
    
    inputFile.open(arpFilename.c_str());
    if (inputFile.fail()){
        cout << "Error opening " << arpFilename << ".\n";
        return "0";
    }
    tempstr.str(string());
    tempstr << "ping 192.168." << node << "0.100 -w 1 >/dev/null";//ping lxc container to ensure there is an entry in the ARP table
    system(tempstr.str().c_str());
    while (!inputFile.eof()){
        //clear string stream
        tempstr.str(string());
        tempstr.clear();
        //Get line of file for processing
        getline(inputFile,line);
        tempstr << line;
        //store IP field for processing
        tempstr >> ipaddr[0];
        //store remainder of line to free string stream
        getline(tempstr,line);
        tempstr.str(string());
        tempstr.clear();
        //divide IP address into octets
        tempstr << ipaddr[0];
        for(i=0;i<ipaddr.size();i++){
            getline(tempstr,ipaddr[i],'.');
        }
        //check if this table entry is for the LXC container
        if (192==atof(ipaddr[0].c_str()) && 168==atof(ipaddr[1].c_str()) && (node*10)==atof(ipaddr[2].c_str()) && 100==atof(ipaddr[3].c_str())){
            //retrieve the remainder of the line
            tempstr.str(string());
            tempstr.clear();
            tempstr << line;
            
            tempstr >> dump;//discard 'HW type' field
            tempstr >> dump;//discard 'flags' field
            tempstr >> lxcmac;//store 'HW address' field for LXC container
            break;
        }
    }
    if (lxcmac==""){
        cout << "MAC address of LXC container not found :(\n";
        return "0";
    }
    inputFile.close();
    
    return lxcmac;
}