/*** LIBRARIES ***/
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
    if ( (nodes + low)->id == id ) return (signed long)low;
    if ( (nodes + high)->id == id ) return (signed long)high;
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
        n = binary_search(nodes, id_n, 0, nnodes-1);
        m = binary_search(nodes, id_m, 0, nnodes-1);
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
        n = binary_search(nodes, id_n, 0, nnodes-1);
        m = binary_search(nodes, id_m, 0, nnodes-1);
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

/*** insert_to_OPEN() takes in a node with a given index and inserts it in the OPEN list. It dynamically allocates memory for the node in the OPEN list and inserts it by preserving the ordering of the f value. It computes such f value by reading g and h values from the progress vector. ***/
/* using theorem 11 in page 84, and assuming we are using a MONOTONE heuristic function, we can only consider cases where a successor node is inserted in the middle of the OPEN or at the end */
void insert_to_OPEN (unsigned long index, AStarStatus* progress, open_node* OPEN) {
    (progress + index)->whq = 1;
    open_node* TEMP = OPEN;                                                                 // copy of the OPEN list
    open_node* new_node = NULL;                                                             // new node in the OPEN list
    if ((new_node = (open_node*) malloc(sizeof(open_node))) == NULL) ExitError("when allocating memory for a new node in the OPEN list", 13);
    new_node->index = index;                                                                // index of new_node
    new_node->f = (progress + index)->g + (progress + index)->h;                            // f function of new node
    new_node->next = NULL;                                                                  // for the moment new node is not allocated in the OPEN list
    while ( (TEMP->next != NULL) && ((TEMP->next)->f < new_node->f ) ) TEMP = TEMP->next;   // find position in OPEN (TEMP) list to insert new_node
    if (TEMP->next == NULL) TEMP->next = new_node;                                          // if new_node has to be allocated at the end of OPEN
    else if ((TEMP->next)->f >= new_node->f) {                                              // if new_node has to be allocated in the middle of OPEN
        new_node->next = TEMP->next;
        TEMP->next = new_node;
    }
}

void delete_from_OPEN (unsigned long index, AStarStatus* progress, open_node* OPEN) {
    open_node* TEMP = OPEN;
    while (TEMP->next != NULL) if ((TEMP->next)->index == index) break;
    if ((TEMP->next == NULL) && (TEMP->index != index)) ExitError("node to be deleted from OPEN list was not found", 13);
    //else if ((TEMP->next == NULL) && (TEMP->index == index))
    TEMP->next = (TEMP->next)->next;
    free(TEMP->next);
}


/* Auxiliary function to keep track of the OPEN list. Used for debugging. Recommended to use only when computing the route between two very close-by nodes. */
void print_OPEN (open_node* OPEN) {
    open_node* TEMP = OPEN;
    printf("\nOPEN list:\nNode index\tf\n");
    while (TEMP != NULL) {
        printf("%lu\t\t%f\n", (TEMP)->index, TEMP->f);
        TEMP = TEMP->next;
    }
    printf("\n");
}











