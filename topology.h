//Adapted from <http://rosettacode.org/wiki/Dijkstra's_algorithm>
//Source code retrieved 05OCT15
#ifndef TOPOLOGY_H
#define TOPOLOGY_H
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <limits> 
#include <set>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <iomanip>
#include <queue>
#include <sstream>
#include <fstream>
using namespace std;

typedef int vertex_t;
typedef double weight_t;
 
const weight_t max_weight = numeric_limits<double>::infinity();
 
struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
};
 
typedef vector<vector<neighbor> > adjacency_list_t;
 

void DijkstraComputePaths(vertex_t source, const adjacency_list_t &adjacency_list, vector<weight_t> &min_distance, vector<vertex_t> &previous);

list<vertex_t> DijkstraGetShortestPathTo(vertex_t vertex, const vector<vertex_t> &previous);

int topology_select(vector<double> traffic, adjacency_list_t, string configFilename, int argc);

vector< vector< list <vertex_t> > > path_calc(adjacency_list_t);

vector<double> calc_link_traffic(vector<double> traffic, vector< vector< list <vertex_t> > > path);

vector<double> calc_link_util(vector<double> linkTraffic, adjacency_list_t adjacency_list);

void display_traffic(vector<double> linkTraffic, vector<double> linkUtil, vector<double> nodeLoad);

#endif