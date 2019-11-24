#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*** MACROS ***/
#define R 6371          // Earth radius in km
#define pi 4*atan(1)    // pi number

/*** structure to represent a node ***/
typedef struct {
    unsigned long id;           // node identification
    char* name;                 // node name (missing for most nodes)
    double lat, lon;            // node position
    unsigned short nsucc;       // number of node successors; i. e. length of successors
    unsigned long* successors;  // list of successors
} node;

/*** memory models and queues for Astar ***/
typedef char Queue;
enum whichQueue {NONE, OPEN, CLOSED};

typedef struct {
    double g, h;                // Dijkstra and heuristic function, such that f = g + h (minimize f)
    unsigned long parent;       // position in nodes vector of predecessor node
    Queue whq;                  // which 
} AStarStatus;


/*** Standard function to exit with error ***/
void ExitError(const char *miss, int errcode) {
    fprintf (stderr, "\nERROR: %s.\nStopping...\n\n", miss); exit(errcode);
}

/*** binary_search() returns the position of node with id in vector nodes. Boundary indices are specified, typically low = 0 and high = nnodes. If node with id is not in the nodes vector, function returns -1. ***/
signed long binary_search(node* nodes, unsigned long id, unsigned long low, unsigned long high) {
    signed long index = (high + low)/2;
    unsigned long guess = (nodes + index)->id;
    while ( (high-low) > 1 ) {
        if (guess == id) { return index; }
        if (guess > id) { high = index; }
        if (guess < id) { low = index; }
        index = (high + low)/2;
        guess = (nodes + index)->id;
    }
    return -1;  // return -1 if element is not in vector
}

/*** Heuristic functions: http://movable-type.co.uk/scripts/latlong.html ***/
double haversine (node u, node v) {
    double diff_lat = (u.lat - v.lat) * pi / 180.f;
    double diff_lon = (u.lon - v.lon) * pi / 180.f;
    double a = pow(sin(diff_lat/2), 2) + cos(u.lat * pi / 180.f) * cos(v.lat * pi / 180.f) * pow(sin(diff_lon/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = R * c;
    return d;
}

void AStar (node* nodes, unsigned long source, unsigned long dest, unsigned long nnodes) {
    
    AStarStatus* progress;
    if ((progress = (AStarStatus*) malloc(nnodes*sizeof(AStarStatus))) == NULL)
        ExitError("when allocating memory for the progress vector", 13);

    unsigned long i;
    for (i=0; i < nnodes; i++) (progress+i)->whq = 0;                               // all nodes start neither in OPEN nor CLOSED list
    
    unsigned long source_index = binary_search(nodes, source, 0, nnodes);           // find source in nodes vector
    unsigned long dest_index = binary_search(nodes, dest, 0, nnodes);               // find destination in nodes vector
    (progress + source_index)->whq = 1;                                             // put source in OPEN list
    (progress + source_index)->g = 0;
    (progress + source_index)->h = haversine( *(nodes+source_index), *(nodes+dest_index) );
    
    
    free(progress);
}
