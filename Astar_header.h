#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

/*** MACROS ***/
#define R 6371          // Earth radius in km
#define pi 4*atan(1)    // pi number

/*** structure to represent a node ***/
typedef struct {
    unsigned long id;           // Node identification
    char* name;                 // Node name (missing for most nodes)
    double lat, lon;            // Node position
    unsigned short nsucc;       // Number of node successors; i. e. length of successors
    unsigned long* successors;  // List of successors
} node;

/*** memory models and queues for Astar ***/
typedef char Queue;
enum whichQueue {NONE, OPEN, CLOSED};

typedef struct {
    double g, h;                // Dijkstra and heuristic function, such that f = g + h (minimize f)
    unsigned long parent;       // position in nodes vector of predecessor node
    Queue whq;                  // which 
} AStarStatus;

/*** structure to represent a node in the OPEN list ***/
typedef struct open_node {
    double f;
    unsigned long index;
    struct open_node* next;
} open_node;

/*** Standard function to exit with error ***/
void ExitError(const char *miss, int errcode) {
    fprintf (stderr, "\nERROR: %s.\nStopping...\n\n", miss); exit(errcode);
}

/*** proces_node(): takes in nodes vector, line from .csv file that is being processed (must be of nodes type) and i position of node in nodes vector. It stores in nodes[i] the id, name, alt and lon. ***/
void process_node(node* nodes, char* line, unsigned long i) {
    char* field = NULL;
    field = strsep(&line, "|");                                              // get first field (node)
    if (*field != 'n') return;                                               // exit if we are not processing a node line
    field = strsep(&line, "|");                                              // get second field (id)
    char* ptr = NULL;                                                        // value of this pointer is set by the strtoul() function
    (nodes+i)->id = strtoul(field, &ptr, 10);
    field = strsep(&line, "|");                                              // get third field (name)
    if (((nodes+i)->name = (char*) malloc(strlen(field) + 1)) == NULL) ExitError("when allocating memory for a node name", 5);
    strcpy( (nodes+i)->name, field );
    unsigned short count;
    for (count = 4; count < 11; count++) field = strsep(&line, "|");         // get 10th field (lat)
    (nodes+i)->lat = atof(field);
    field = strsep(&line, "|");                                              // get 11th field (lon)
    (nodes+i)->lon = atof(field);
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

/*** update_nsuccs() updates nsuccdim with the information provided in a line (must be of way type) of the .csv file. ***/
void update_nsuccs(unsigned short* nsuccdim, char* line, node* nodes, unsigned long nnodes) {
    bool oneway = false;                            // by default, way is bidirectional
    char* field = strsep(&line, "|");               // read first field of line
    if (*field != 'w') return;                      // if line is not way: exit
    unsigned short count;
    for (count = 2; count < 9; count++) field = strsep(&line, "|");       // get 8th field: oneway                  
    if (*field == 'o') oneway = true;                // change way to unidirectional if it is so
    field = strsep(&line, "|");                      // unimportant 9th field
    
    char* ptr = NULL;                                // value of this pointer is set by the strtoul() function
    char* node_n = NULL;                             // string read from line of first node
    char* node_m = NULL;                             // string read from line of second node
    unsigned long id_n, id_m;                        // id's of first and second node
    signed long n, m;                                // position of first and second node in nodes vector
    node_n = strsep(&line, "|");                     // always at least one node in way
    while ((node_m = strsep(&line, "|")) != NULL) {  // read current field
        id_n = strtoul(node_n, &ptr, 10);
        id_m = strtoul(node_m, &ptr, 10);
        n = binary_search(nodes, id_n, 0, nnodes);
        m = binary_search(nodes, id_m, 0, nnodes);
        node_n = node_m;                             // advance to next pair of consecutive nodes
        if ( (n == -1) || (m == -1) ) continue;      // if any of the nodes is not in nodes vector -> skip
        else if (oneway == true) nsuccdim[n] += 1;
        else { nsuccdim[n] += 1; nsuccdim[m] += 1; }
    }
}

/*** update_successors() reads a line of type way from the .csv file and updates the adjacency list accordingly. ***/
void update_successors(char* line, node* nodes, unsigned long nnodes, unsigned short* counters) {
    bool oneway = false;                            // by default, way is bidirectional
    char* field = strsep(&line, "|");               // read first field of line
    if (*field != 'w') return;                      // if line is not way: exit
    unsigned short count;
    for (count = 2; count < 9; count++) field = strsep(&line, "|");       // get 8th field: oneway                  
    if (*field == 'o') oneway = true;                // change way to unidirectional if it is so
    field = strsep(&line, "|");                      // unimportant 9th field
    
    char* ptr = NULL;                                // value of this pointer is set by the strtoul() function
    char* node_n = NULL;                             // string read from line of first node
    char* node_m = NULL;                             // string read from line of second node
    unsigned long id_n, id_m;                        // id's of first and second node
    signed long n, m;                                // position of first and second node in nodes vector
    unsigned short free_n, free_m;                   // position of first free successor nodes in the adjacency list of each node
    node_n = strsep(&line, "|");                     // always at least one node in way
    while ((node_m = strsep(&line, "|")) != NULL) {  // read another node
        id_n = strtoul(node_n, &ptr, 10);
        id_m = strtoul(node_m, &ptr, 10);
        n = binary_search(nodes, id_n, 0, nnodes);
        m = binary_search(nodes, id_m, 0, nnodes);
        node_n = node_m;                              // first node is now second node to read next pair 
        if ( (n == -1) || (m == -1) ) continue;       // if any of the nodes is not in nodes vector -> skip
        free_n = counters[n];                         // find free spot for successor of node_n
        free_m = counters[m];                         // find free spot for successor of node_m 
        if (oneway == true) { ((nodes+n)->successors)[free_n] = m;  counters[n] += 1; } // write successor  m(n) in adjacency list of n(m)
        if (oneway == false) {                                                          // and increase the counters to next position in adjacency list
            ((nodes+n)->successors)[free_n] = m;    counters[n] += 1;
            ((nodes+m)->successors)[free_m] = n;    counters[m] += 1;
        }
    }
}

void delete_duplicates(node* nodes, unsigned long nnodes, unsigned short* nsuccdim) {
    unsigned long i;
    unsigned short j, k;
    signed long* aux = NULL;
    for (i = 0; i < nnodes; i++) {
        if((aux = (signed long*) malloc(nsuccdim[i]*sizeof(signed long))) == NULL) ExitError("when allocating memory for auxiliary copy of successors", 5);
        for (j = 0; j < (nodes+i)->nsucc; j++) {
            for (k = 0; k < j; k++) {
                if ( ((nodes+i)->successors)[j] == ((nodes+i)->successors)[k] ) { *aux = -1; aux++; break; }
            }
        }
        free(aux);
    }
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

void insert_to_OPEN (unsigned long index, AStarStatus* progress, open_node* OPEN) {
    (progress + index)->whq = 1;
    struct open_node* new_open_node = NULL;
    if ((new_open_node = (open_node*) malloc(sizeof(open_node))) == NULL)
        ExitError("when allocating memory for a new node in the OPEN list", 13);
    new_open_node->f = (progress + index)->g + (progress + index)->h;
    new_open_node->index = index;
    new_open_node->next = NULL;
    
// if the node has to be inserted at the front of the queue
    if ( new_open_node->f <= OPEN->f ) {
        new_open_node->next = OPEN;
        OPEN = new_open_node;
        return;
    }
    
// if the node has to be inserted in the middle or end of the queue
    struct open_node* TEMP = OPEN;
    while ( ((TEMP->next)->f < new_open_node->f) && (TEMP->next != NULL) ) TEMP = TEMP->next;
// if we have reached the end of the queue
    if (TEMP->next == NULL) TEMP->next = new_open_node;
    else {
        new_open_node->next = TEMP->next;
        TEMP->next = new_open_node;
    }
}
