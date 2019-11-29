/*** LIBRARIES ***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

/*** MACROS ***/
#define R 6371          // Earth radius in km
#define pi 4*atan(1)    // pi number

/*** structure to represent a node ***/
typedef struct {
    unsigned long id;           // Node identification
    unsigned short namelen;     // Length of name in number of characters
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
    nodes[i].id = strtoul(field, &ptr, 10);
    field = strsep(&line, "|");                                              // get third field (name)
    nodes[i].namelen = strlen(field);
    if ((nodes[i].name = (char*) malloc(nodes[i].namelen + 1)) == NULL) ExitError("when allocating memory for a node name", 4);
    strcpy( nodes[i].name, field );
    unsigned short count;
    for (count = 4; count < 11; count++) field = strsep(&line, "|");         // get 10th field (lat)
    nodes[i].lat = atof(field);
    field = strsep(&line, "|");                                              // get 11th field (lon)
    nodes[i].lon = atof(field);
}

/*** binary_search() returns the position of node with id in vector nodes. Boundary indices are specified, typically low = 0 and high = nnodes. If node with id is not in the nodes vector, function returns -1. ***/
signed long binary_search(node* nodes, unsigned long id, unsigned long low, unsigned long high) {
    if ( nodes[low].id == id ) return (signed long)low;
    if ( nodes[high].id == id ) return (signed long)high;
    signed long index = (high + low)/2;
    unsigned long guess = nodes[index].id;
    while ( (high-low) > 1 ) {
        if (guess == id) { return index; }
        if (guess > id) { high = index; }
        if (guess < id) { low = index; }
        index = (high + low)/2;
        guess = nodes[index].id;
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
        if (oneway == true) { (nodes[n].successors)[free_n] = m;  counters[n] += 1; } // write successor  m(n) in adjacency list of n(m)
        else {                                                                        // and increase the counters to next position in adjacency list
            (nodes[n].successors)[free_n] = m;    counters[n] += 1;
            (nodes[m].successors)[free_m] = n;    counters[m] += 1;
        }
    }
}

/*** Haversine distance: great-circle distance between two points. ***/
double haversine (node u, node v) {
    double diff_lat = (u.lat - v.lat) * pi / 180.f;
    double diff_lon = (u.lon - v.lon) * pi / 180.f;
    double a = pow(sin(diff_lat/2), 2) + cos(u.lat * pi / 180.f) * cos(v.lat * pi / 180.f) * pow(sin(diff_lon/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = R * c;
    return d;
}

double evaluation_function (int mode, double param, AStarStatus* info, node* nodes, unsigned long cur_index, unsigned long src_index, unsigned long dest_index) {
// default evaluation
    if (mode == 1) return info[cur_index].g + info[cur_index].h;
// weighted evaluation
    else if (mode == 2) return (1-param)*info[cur_index].g + param*info[cur_index].h;
// dynamic weighting
    else if (mode == 3) {
        double extra = 1 - haversine(nodes[cur_index], nodes[src_index]) / (haversine(nodes[cur_index], nodes[dest_index]) + haversine(nodes[cur_index], nodes[src_index]));
        return info[cur_index].g + info[cur_index].h + param*extra*info[cur_index].h;
    }
    ExitError("Invalid choice of evaluation function.", 22);
    return 0.f;
}

/*** Auxiliary function to keep track of the OPEN list. Used for debugging. Recommended to use only when testing the algorithm to compute the route between two close-by nodes. ***/
void print_OPEN (open_node* OPEN) {
    open_node* TEMP = OPEN;
    printf("\nOPEN list:\nNode index\tf\n");
    while (TEMP != NULL) {
        printf("%lu\t\t%f\n", (TEMP)->index, TEMP->f);
        TEMP = TEMP->next;
    }
    printf("\n");
}

/*** insert_to_OPEN() takes in a node with a given index and inserts it in the OPEN list. It dynamically allocates memory for the node in the OPEN list and inserts it by preserving the ordering of the f value. It computes such f value by reading g and h values from the progress vector. Tie break rule: a new node that is inserted and has the same f as a node already in the OPEN list. Then, the incoming node is inserted behind the node already in the OPEN list. ***/
void insert_to_OPEN (unsigned long index, AStarStatus* progress, open_node* OPEN, node* nodes, int mode, double param, unsigned long src_index, unsigned long dest_index) {
    (progress + index)->whq = 1;
    open_node* TEMP = OPEN;                                                                 // copy of the OPEN list
    open_node* new_node = NULL;                                                             // new node in the OPEN list
    if ((new_node = (open_node*) malloc(sizeof(open_node))) == NULL) ExitError("when allocating memory for a new node in the OPEN list", 21);
    new_node->index = index;                                                                // index of new_node
    new_node->f = evaluation_function (mode, param, progress, nodes, index, src_index, dest_index); // f function of new node
    new_node->next = NULL;                                                                  // for the moment new node is not allocated in the OPEN list
    while ( (TEMP->next != NULL) && ((TEMP->next)->f <= new_node->f ) ) TEMP = TEMP->next;  // find position in OPEN (TEMP) list to insert new_node
    if (TEMP->next == NULL) TEMP->next = new_node;                                          // if new_node has to be allocated at the end of OPEN
    else if ((TEMP->next)->f > new_node->f) {                                               // if new_node has to be allocated in the middle of OPEN
        new_node->next = TEMP->next;
        TEMP->next = new_node;
    }
}

/*** delete_from_OPEN() deletes a node from the OPEN list, when we find that its cost is cheaper after reaching it from the current node. It will never delete the first node in the OPEN list because the first node in OPEN is the node being currently expanded, which cannot be a successor of itself. ***/
void delete_from_OPEN (unsigned long target, AStarStatus* progress, open_node* OPEN) {
    open_node* TEMP = OPEN;
    open_node* PREV = NULL;
    while (TEMP != NULL && TEMP->index != target) {
        PREV = TEMP;
        TEMP = TEMP->next;
    }
    PREV->next = TEMP->next;
    free(TEMP);
}

/*** is_path_correct() checks that a computed path makes sense. Given a sequence of unsigned long indices, check that for all i, element (i+1) is a successor of element i. ***/
bool is_path_correct (unsigned long* path, node* nodes) {
    unsigned long current;
    unsigned long next;
    size_t len = sizeof(path) / sizeof(unsigned long);
    unsigned long i;
    unsigned short j;
    for (i = 0; i < len-1; i++) {
        current = path[i];
        next = path[i+1];
        for (j = 0; j < nodes[current].nsucc; j++) if ( (nodes[current].successors)[j] == next ) break;
        if (j == nodes[current].nsucc) return false;
    }
    return true;
}

/*** path_to_file() creates an output file with the sequence of nodes that make up the path. For each node in the path, the following information is written: ID, lat, lon, g, h, f and name. ***/
void path_to_file(node* nodes, unsigned long* path, unsigned long length, AStarStatus* info, char* name, int evaluation, double param) {
// modify name to the following format (map)_(id of source)_(id of destination).csv. Map is either spain or cataluna
    char ending[257] = "_";
    char buffer[11];
    sprintf(buffer, "%lu", nodes[path[0]].id);
    strcat(ending, buffer);
    strcat(ending, "_");
    sprintf(buffer, "%lu", nodes[path[length-1]].id);
    strcat(ending, buffer);
    if (evaluation == 1) strcat(ending, "_default");
    else if (evaluation == 2) {
        strcat(ending, "_weighted_");
        sprintf(buffer, "%.2f", param);
        strcat(ending, buffer);
    }
    else if (evaluation == 3) {
        strcat(ending, "_dynamic");
        sprintf(buffer, "%.2f", param);
        strcat(ending, buffer);
    }
    else ExitError("Invalid choice of evaluation function.", 26);     
    strcat(ending, ".csv");
    strcpy(strrchr(name, '.'), ending);
    FILE *fout;
    if ((fout = fopen (name, "w+")) == NULL) ExitError("the output data file cannot be created", 27);   
    fprintf(fout, "step|id|lat|lon|g|h|name\n");
    unsigned long i;
    for (i = 0; i < length; i++) {
        fprintf(fout, "%lu|%lu|%.7f|%.7f|%.7f|%.7f|%s\n", i, nodes[path[i]].id, nodes[path[i]].lat, nodes[path[i]].lon, info[path[i]].g, info[path[i]].h, nodes[path[i]].name);
    }
    
    fclose(fout);
}
