#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

/*** structure to represent a node ***/
typedef struct {
    unsigned long id;           // Node identification
    char* name;                 // Node name (missing for most nodes)
    double lat, lon;            // Node position
    unsigned short nsucc;       // Number of node successors; i. e. length of successors
    unsigned long* successors;  // List of successors
} node;

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
