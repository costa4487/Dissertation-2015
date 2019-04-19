#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <iomanip>
using namespace std;

int main(){
    int NUM_NODES = 8;
    int FIELDS = 14;
    int JITTER = 9;
    int LOSS = 12;
    int ORDER = 13;
    int i, j, k, m;
    ifstream inputFile;
    stringstream tempstr;
    string line, temp, dump;
    vector<double> delay(NUM_NODES*NUM_NODES);
    vector<double> jitter(NUM_NODES*NUM_NODES);
    vector<double> loss(NUM_NODES*NUM_NODES);
    vector<double> order(NUM_NODES*NUM_NODES);
    
    vector< vector<double> > delayFull, jitterFull, lossFull, orderFull;
    
    for (i=0;i<3;i++){
        for (j=0;j<NUM_NODES;j++){
            for (k=0;k<NUM_NODES;k++){
                if (j!=k){
                    //Get statistics from iperf files
                    tempstr.str(string());
                    tempstr.clear();
                    tempstr << "./" << (j+1) << "-" << (k+1) << "_" << (i+1) << ".iperf";
                    inputFile.open(tempstr.str().c_str());
                    if (inputFile.fail()){
                        cout << "Failed to open file " << tempstr.str() << "\n\n";
                        return 1;
                    }
                    
                    tempstr.str(string());
                    tempstr.clear();
                    while(getline(inputFile,line)){temp = line;}
                    tempstr << temp;
                    for (m=0;m<FIELDS;m++){
                        getline(tempstr,temp,',');
                        if (m==JITTER) jitter[NUM_NODES*j + k] = atof(temp.c_str());
                        if (m==LOSS) loss[NUM_NODES*j + k] = atof(temp.c_str());
                        if (m==ORDER) order[NUM_NODES*j + k] = atof(temp.c_str());
                    }
                    inputFile.close();
                    
                    //Get statistics from ping files
                    tempstr.str(string());
                    tempstr.clear();
                    tempstr << "./" << (j+1) << "-" << (k+1) << "_" << (i+1) << ".ping";
                    inputFile.open(tempstr.str().c_str());
                    if (inputFile.fail()){
                        cout << "Failed to open file " << tempstr.str() << "\n\n";
                        return 1;
                    }
                    tempstr.str(string());
                    tempstr.clear();
                    while(getline(inputFile,line)){temp = line;}
                    tempstr << temp;
                    
                    tempstr >> temp;//discard unwanted data
                    tempstr >> temp;//discard unwanted data
                    tempstr >> temp;//discard unwanted data
                    getline(tempstr,temp,'/');//discard unwanted data
                    
                    getline(tempstr,temp,'/');//get the average round trip time
                    delay[NUM_NODES*j + k] = atof(temp.c_str());
                    
                    inputFile.close();
                }
            }
        }
        delayFull.push_back(delay);
        jitterFull.push_back(jitter);
        lossFull.push_back(loss);
        orderFull.push_back(order);
        for(j=0;j<delay.size();j++){
            delay[j] = 0;
            jitter[j] = 0;
            loss[j] = 0;
            order[j] = 0;
        }
    }
    
    for (i=0;i<delayFull.size();i++){
        cout << "= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
        cout << " SCENARIO " << i+1 << "\n";
        cout << "= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
        cout << "Average round trip time (ms):\n";
        for (j=0;j<delayFull[i].size();j++){
            cout << "\t" << (int)(j/NUM_NODES)+1 << "-" << (j%NUM_NODES)+1 << ": " << setw(5) << delayFull[i][j];
            if (j%NUM_NODES==7)
                cout << "\n";
        }
        cout << "\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
        cout << "Jitter (ms):\n" << setprecision(2);
        for (j=0;j<jitterFull[i].size();j++){
            cout << "\t" << (int)(j/NUM_NODES)+1 << "-" << (j%NUM_NODES)+1 << ": " << setw(5) << jitterFull[i][j];
            if (j%NUM_NODES==7)
                cout << "\n";
        }
        cout << "\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
        cout << "Packet loss (%):\n";
        for (j=0;j<lossFull[i].size();j++){
            cout << "\t" << (int)(j/NUM_NODES)+1 << "-" << (j%NUM_NODES)+1 << ": " << setw(5) << lossFull[i][j];
            if (j%NUM_NODES==7)
                cout << "\n";
        }
        cout << "\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
        cout << "Packets received out of order (count):\n";
        for (j=0;j<orderFull[i].size();j++){
            cout << "\t" << (int)(j/NUM_NODES)+1 << "-" << (j%NUM_NODES)+1 << ": " << setw(5) << orderFull[i][j];
            if (j%NUM_NODES==7)
                cout << "\n";
        }
    }
    return 0;
}