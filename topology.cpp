//Adapted from <http://rosettacode.org/wiki/Dijkstra's_algorithm>
//Source code retrieved 05OCT15

#include "topology.h"
using namespace std;

int topology_select(vector<double> traffic, adjacency_list_t adjacency_list, string configFilename, int args) {
    unsigned int NUM_NODES = adjacency_list.size();
    unsigned int i,j,k;
    int leastUsed, mostUsed, targetNode, options;
    vector<int> transitUsage(NUM_NODES);
    list<vertex_t>::iterator pos;
    double minUsage = numeric_limits<double>::infinity();
    double minLoad = numeric_limits<double>::infinity();
    int maxUsage = 0;
    int maxLoad = 0;
    bool connected = 0;
    vector< vector<neighbor> > nodeStorage;

    double THRESHOLD = 70;//link utilisation threshold
    double maxUtil;
    vector<double> linkTraffic(NUM_NODES*NUM_NODES);//traffic on each link in Mbps
    vector<double> linkUtil(NUM_NODES*NUM_NODES);//traffic on each link in % utilisation



    vector< vector< list <vertex_t> > > path(NUM_NODES);//paths between each S/D node pair (bidirectional)
    vector<double> nodeLoad(NUM_NODES);//sum of each node's connected link's traffic
    vector<bool> active(NUM_NODES);

    stringstream outputString;
    fstream config;
    
    
    //Initial path computation with all nodes present/active
    for (i=0;i<NUM_NODES;i++) active[i]=1;

    path = path_calc(adjacency_list);

    //Calculate link usage in Mbps from traffic matrix and paths
    linkTraffic = calc_link_traffic(traffic, path);

    //Calculate node load
    for (i=0;i<linkTraffic.size();i++) nodeLoad[i/NUM_NODES] += linkTraffic[i];

    //Calculate link utilisation percentage
    linkUtil = calc_link_util(linkTraffic, adjacency_list);

    //Find max link utilisation percentage
    maxUtil = 0;
    for (i=0;i<linkUtil.size();i++){
        if (linkUtil[i] > maxUtil)
            maxUtil = linkUtil[i];
    }
        
    //Skip optimisation if dynamic mechanism is disabled (defined on command line)
    if (args==1){
        while (maxUtil < THRESHOLD){
            connected = 0;
            options = 0;
            for (i=0;i<NUM_NODES;i++){
                options += active[i];
            }
            if (options == 0) break;

            //Clear residual node traffic data and calculate node load
            for (i=0;i<nodeLoad.size();i++) nodeLoad[i] = 0;
            for (i=0;i<linkTraffic.size();i++) nodeLoad[i/NUM_NODES] += linkTraffic[i];

            //Until all nodes are connected
            while(!connected || maxUtil > THRESHOLD){
                options = 0;
                for (i=0;i<NUM_NODES;i++){
                    options += active[i];
                }
                if (options == 0) break;
                //Clear residual data
                for (i=0;i<NUM_NODES;i++) transitUsage[i] = 0;

                //Determine how many times each node is used in a shortest path
                for (i=0;i<NUM_NODES;i++){
                    for (j=0;j<NUM_NODES;j++){
                        for (pos=path[i][j].begin();pos!=path[i][j].end();pos++){
                            transitUsage[*pos]++;
                        }
                    }
                }
                
                //find least used node
                minUsage = numeric_limits<double>::infinity();
                minLoad = numeric_limits<double>::infinity();
                leastUsed = 9999;
                for (i=0;i<NUM_NODES;i++){
                    if (active[i] && transitUsage[i] <= minUsage){
                        minUsage = transitUsage[i];
                        leastUsed = i;
                    }
                    else if (active[i] && transitUsage[i] == minUsage && nodeLoad[i] < minLoad){
                        minLoad = nodeLoad[i];
                        leastUsed = i;
                    }
                }
                if (leastUsed == 9999){//no candidates for removal
                    break;
                }
                //Remove the node as a candidate for future removal
                active[leastUsed] = 0;
                options--;

                //Store adjacency information in case node removal has to be reversed
                nodeStorage.push_back(adjacency_list[leastUsed]);
                for (i=0;i<adjacency_list[leastUsed].size();i++){
                    nodeStorage.push_back(adjacency_list[adjacency_list[leastUsed][i].target]);
                }
                
                //find most used adjacency to least used node
                maxUsage = 0;
                maxLoad = 0;
                mostUsed = 9999;
                for (i=0;i<adjacency_list[leastUsed].size();i++){
                    if (transitUsage[adjacency_list[leastUsed][i].target] > maxUsage){
                        maxUsage = transitUsage[adjacency_list[leastUsed][i].target];
                        mostUsed = adjacency_list[leastUsed][i].target;
                    }
                    else if (transitUsage[adjacency_list[leastUsed][i].target] == maxUsage && nodeLoad[adjacency_list[leastUsed][i].target] > maxLoad){
                        maxLoad = nodeLoad[adjacency_list[leastUsed][i].target];
                        mostUsed = adjacency_list[leastUsed][i].target;
                    }

                }
                cout << "\tRemoving node " << leastUsed+1 << "\t(Keep node " << mostUsed+1 << " adjacency)\n";

                //Remove all but one adjacency from node to be removed
                for (i=0;i<adjacency_list[leastUsed].size();i++){
                    targetNode = adjacency_list[leastUsed][i].target;
                    if (targetNode != mostUsed){
                        //Remove adjacencies from other nodes
                        for (j=0;j<adjacency_list[targetNode].size();j++){
                            if (adjacency_list[targetNode][j].target == leastUsed){
                                for (k=j ; k<adjacency_list[targetNode].size()-1 ;k++){
                                    adjacency_list[targetNode][k] = adjacency_list[targetNode][k+1];
                                }
                                adjacency_list[targetNode].pop_back();
                                j--;
                            }
                        }
                        //Remove adjacencies from node to be removed
                        for (j=i ; j<adjacency_list[leastUsed].size()-1 ;j++){
                            adjacency_list[leastUsed][j] = adjacency_list[leastUsed][j+1];
                        }
                        adjacency_list[leastUsed].pop_back();
                        i--;
                    }
                }

                //calculate new paths between nodes
                path = path_calc(adjacency_list);

                //Calculate link usage in Mbps from traffic matrix and paths
                linkTraffic = calc_link_traffic(traffic, path);

                //Calculate link utilisation percentage
                linkUtil = calc_link_util(linkTraffic, adjacency_list);

                //Find max link utilisation percentage
                maxUtil = 0;
                for (i=0;i<linkUtil.size();i++){
                    if (linkUtil[i] > maxUtil)
                        maxUtil = linkUtil[i];
                }
                
                //test if each node can reach all other nodes
                for(i=0;i<path.size();i++){
                    for (j=0;j<path[i].size();j++){
                        //if a node is unreachable, the path is simply the destination node
                        //Therefore, if a node is reachable, the start of the path will be the source node
                        connected = (i == *(path[i][j].begin()));
                        if (!connected) break;
                    }
                    if (!connected) break;
                }
                
                //Add node back into topology if one of more nodes are unreachable, or if the utilisation is over the threshold
                if (!connected || maxUtil > THRESHOLD) {
                    cout << "\t\tAdding removed node back in";
                    if(maxUtil > THRESHOLD){
                        if (!connected) cout << " (Utilisation over threshold AND one or more nodes unreachable)\n";
                        else cout << " (Utilisation over threshold)\n";
                    }
                    else cout << " (One or more nodes unreachable)\n";
                    //Add the removed node back into the network
                    adjacency_list[leastUsed] = nodeStorage[0];
                    for (i=0;i<adjacency_list[leastUsed].size();i++){
                        adjacency_list[adjacency_list[leastUsed][i].target] = nodeStorage[i+1];
                    }

                    path = path_calc(adjacency_list);
                }
                //Clear the node storage for future use
                while(nodeStorage.size() > 0) nodeStorage.pop_back();
            }

            //Calculate link usage in Mbps from traffic matrix and paths
            linkTraffic = calc_link_traffic(traffic, path);

            //Calculate link utilisation percentage
            linkUtil = calc_link_util(linkTraffic, adjacency_list);

            //Find max link utilisation percentage
            maxUtil = 0;
            for (i=0;i<linkUtil.size();i++){
                if (linkUtil[i] > maxUtil)
                    maxUtil = linkUtil[i];
            }
            
            //If no candidates remain, exit the loop
            if (options == 0) break;
        }
    }
    
    //Write topology to config file
    config.open(configFilename.c_str());
    if (config.fail()){
        cout << "Error opening " << configFilename << "\n";
        return 1;
    }
    for (i=0;i<path.size();i++){
        for (j=0;j<path[i].size();j++){
            if (i!=j){
                pos = path[i][j].begin();
                pos++;
                config << *pos+1;
            }
            else config << i+1;
            config << ",";
        }
        if (i<path.size()-1) config << "\n";
    }
    config.close();
    return 0;
}

vector< vector< list <vertex_t> > > path_calc(adjacency_list_t adjacency_list){
    unsigned int i,j;
    unsigned int NUM_NODES = adjacency_list.size();
    vector<weight_t> min_distance;//minimum distance between S/D node pair (not used)
    vector< vector <vertex_t> > previous(NUM_NODES);//used in path calculation
    vector< vector< list <vertex_t> > > path(NUM_NODES);//paths between each S/D node pair (bidirectional)

    for (i=0;i<NUM_NODES;i++) DijkstraComputePaths(i, adjacency_list, min_distance, previous[i]);

    for (i=0;i<NUM_NODES;i++){
        for (j=0;j<NUM_NODES;j++){
            path[i].push_back(DijkstraGetShortestPathTo(j, previous[i]));
        }
    }
    return path;
}

vector<double> calc_link_traffic(vector<double> traffic, vector< vector< list <vertex_t> > > path){
    unsigned int i, j;
    unsigned int NUM_NODES = (int)sqrt((double)traffic.size());
    vector<double> linkTraffic(NUM_NODES*NUM_NODES);//traffic on each link in Mbps
    list<vertex_t>::iterator pos1, pos2;

    for (i=0;i<NUM_NODES;i++){
        for (j=0;j<NUM_NODES;j++){
            pos1 = path[i][j].begin();
            pos1++;
            pos2 = pos1;
            pos1--;
            while (pos2!=path[i][j].end()){
                linkTraffic[NUM_NODES*(*pos1) + *pos2] += traffic[NUM_NODES*i + j];
                pos1++;
                pos2++;
            }
        }
    }
    return linkTraffic;
}

vector<double> calc_link_util(vector<double> linkTraffic, adjacency_list_t adjacency_list){
    unsigned int NUM_NODES = (int)sqrt((double)linkTraffic.size());
    unsigned int i,j;
    vector<double> linkUtil(NUM_NODES*NUM_NODES);

    for (i=0;i<NUM_NODES;i++){
        for (j=0;j<adjacency_list[i].size();j++){
            linkUtil[NUM_NODES*i + adjacency_list[i][j].target] = (linkTraffic[NUM_NODES*i + adjacency_list[i][j].target] * adjacency_list[i][j].weight) * 10;
        }
    }
    return linkUtil;
}

void DijkstraComputePaths(vertex_t source, const adjacency_list_t &adjacency_list, vector<weight_t> &min_distance, vector<vertex_t> &previous){
    int n = adjacency_list.size();
    min_distance.clear();
    min_distance.resize(n, max_weight);
    min_distance[source] = 0;
    previous.clear();
    previous.resize(n, -1);
    set<pair<weight_t, vertex_t> > vertex_queue;
    vertex_queue.insert(make_pair(min_distance[source], source));

    while (!vertex_queue.empty()){
        weight_t dist = vertex_queue.begin()->first;
        vertex_t u = vertex_queue.begin()->second;
        vertex_queue.erase(vertex_queue.begin());

        // Visit each edge exiting u
        const vector<neighbor> &neighbors = adjacency_list[u];
        for (vector<neighbor>::const_iterator neighbor_iter = neighbors.begin();
             neighbor_iter != neighbors.end();
             neighbor_iter++)
        {
            vertex_t v = neighbor_iter->target;
            weight_t weight = neighbor_iter->weight;
            weight_t distance_through_u = dist + weight;
            if (distance_through_u < min_distance[v]) {
                vertex_queue.erase(make_pair(min_distance[v], v));

                min_distance[v] = distance_through_u;
                previous[v] = u;
                vertex_queue.insert(make_pair(min_distance[v], v));

            }

        }
    }
}

list<vertex_t> DijkstraGetShortestPathTo(
    vertex_t vertex, const vector<vertex_t> &previous)
{
    list<vertex_t> path;
    for ( ; vertex != -1; vertex = previous[vertex])
        path.push_front(vertex);
    return path;
}