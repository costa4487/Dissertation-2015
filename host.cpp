#include <iostream>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

int main(){
    timeval t0, t1;
    struct tm* now;
    struct tm* start;
    bool flag = 1;
    ifstream inputFile;
    stringstream nodestr, tempstr;
    vector<string> trafficFile(3);
    vector< vector<double> > traffic(3);
    string line, trafficstr;
    string controllerURL = "ftp://192.168.254.100/";
    int node,i,j;
    int interval = 10*60;//testing interval in seconds
    char tempchar;
    
    //Traffic generation matrix filenames
    trafficFile[0] = "./1.matrix";
    trafficFile[1] = "./2.matrix";
    trafficFile[2] = "./3.matrix";

    //determine node number
    inputFile.open("/etc/hostname");
    if (inputFile.fail()){
        cout << "Error opening /etc/hostname.\n";
        return 1;
    }
    inputFile.seekg(-1,ios::end);
    inputFile.get(tempchar);
    nodestr << tempchar;
    nodestr >> node;
    inputFile.close();
    
    //get this node's traffic from matrix files
    for (i=0;i<3;i++){
        inputFile.open((trafficFile[i]).c_str());
        for (j=0;j<8;j++){
            getline(inputFile,line);
            if(j==node-1) break;
        }
        tempstr.str(string());//clear string stream for use
        tempstr.clear();
        tempstr << line;
        for (j=0;j<8;j++){
            getline(tempstr,trafficstr,',');
            traffic[i].push_back(atof(trafficstr.c_str()));
        }
        inputFile.close();
    }
    
    
    //get local time
    gettimeofday(&t0, NULL);
    now = localtime(&t0.tv_sec);
    start = localtime(&t0.tv_sec);
    
    //Output current time
    cout << "Current time: ";
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
    
    //determine test start time - closest 30s interval 
    if (start->tm_sec < 15)
        start->tm_sec = 30;
    else if (start->tm_sec >= 15 && start->tm_sec < 45){
        start->tm_sec = 0;
        start->tm_min++;}
    else if (start->tm_sec >= 45){
        start->tm_sec = 0;
        start->tm_min++;}

    
    if (start->tm_min >= 60){ 
        start->tm_hour++;
        start->tm_min -= 60;
    }
    if (start->tm_hour >= 24)
        start->tm_hour -= 24;
    t1.tv_sec = mktime(start);
    t1.tv_usec = 0;
    
    //Output start time
    cout << "Start time: ";
    if (start->tm_hour < 10) cout << "0";
    cout << start->tm_hour << ":";
    if (start->tm_min < 10) cout << "0";
    cout << start->tm_min << ":";
    if (start->tm_sec < 10) cout << "0";
    cout << start->tm_sec << ".";
    if (t1.tv_usec/1000 < 100) cout << "0";
    if (t1.tv_usec/1000 < 10) cout << "0";
    cout << t1.tv_usec/1000 << "\n";

    //wait until local time == start time
    cout << "Waiting for test start time\n" << ((t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000) << " seconds until start";
    while((t1.tv_sec - t0.tv_sec)*1000 + (t1.tv_usec - t0.tv_usec)/1000 > 0){
        gettimeofday(&t0, NULL);
        if (int((t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000) % 30 == 0){
            if (flag){
                cout << "\n" << (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec)/1000000 << " seconds until start";
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
    
    //start iperf server for traffic reception
    tempstr.str(string());//clear string stream for use
    tempstr << "iperf -s -p 5000" << (node-1) << " -B 192.168." << node << "0.100 -u &";
    system(tempstr.str().c_str());

    tempstr.str(string());//clear string stream for use
    tempstr << "iperf -s -p 5001" << (node-1) << " -B 192.168." << node << "0.100 -u &";
    system(tempstr.str().c_str());

    tempstr.str(string());//clear string stream for use
    tempstr << "iperf -s -p 5002" << (node-1) << " -B 192.168." << node << "0.100 -u &";
    system(tempstr.str().c_str());

    for (i=0;i<traffic.size();i++){
        for (j=0;j<traffic[i].size();j++){
            if (j!=(node-1)){
                //start iperf clients for traffic generation
                tempstr.str(string());//clear string stream for use
                tempstr.clear();
                tempstr << "iperf -c 192.168." << (j+1)*10 << ".100 -p 500" << i << j << " -B 192.168." << node << "0.100 -t " << interval*(3-i) << " -b " << traffic[i][j] << "M -u -y c >./" << node << "-" << j+1 << "_" << i+1 << ".iperf &";
                system(tempstr.str().c_str());
                
                //start ping
                tempstr.str(string());//clear string stream for use
                tempstr.clear();
                tempstr << "ping 192.168." << (j+1)*10 << ".100 -w " << interval << " -q >./" << node << "-" << j+1 << "_" << i+1 << ".ping &";
                system(tempstr.str().c_str());
            }
        }
        //wait until test finishes
        usleep(1000*1000*interval);
    }
    cout << "\n==============================================\n";
    cout << "TEST FINISHED";
    cout << "===============================================\n";
    return 0;
}