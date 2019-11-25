#include "Astar_header.h"

int main (int argc, char *argv[]) {
    
// Open csv file and select what map we are working with
    FILE *fmap;
    if ((fmap = fopen(argv[1], "r")) == NULL) ExitError("when opening the csv file", 2);
    unsigned long nnodes;
    if (*(argv[1]) == 's') nnodes = 23895681UL; // nodes in Spain map
    if (*(argv[1]) == 'c') nnodes = 3474545UL;  // nodes in Catalonia map
    
// Vector of nodes
    node *nodes;
    if((nodes = (node *) malloc(nnodes*sizeof(node))) == NULL) ExitError("when allocating memory for the nodes vector", 5);
    unsigned long i = 0;  // keep track of the position in nodes vector of the node being processed

// Vector of number of successors of each node. calloc() initializes allocated memory to 0
    unsigned short* nsuccdim = NULL;
    if ((nsuccdim = (unsigned short*) calloc(nnodes, sizeof(unsigned short))) == NULL) ExitError("when allocating memory for nsuccdim", 5);
    
// Variables related to getline()
    char* line_buf = NULL;      // line that is being read from file
    size_t line_buf_size = 0;   // size of allocated memory for line
    ssize_t line_size = 0;      // size of line in number of characters
    
/*** FIRST READ OF .csv FILE: Lines that correspond to nodes get info on node id, name, lat and lon. Lines that correspond to ways are used to compute the elements of nsuccdim ***/
    while (line_size >= 0) {
        line_size = getline(&line_buf, &line_buf_size, fmap);// read the line
        if (*(line_buf) == '#') continue;                    // skip comment lines
        if (*(line_buf) == 'n') {                            // process a line that corresponds to a node
            process_node(nodes, line_buf, i);                // and move to next position in nodes vector
            i += 1;
            continue;
        }
        if (*(line_buf) == 'w') {                             // process a line that corresponds to a way
            update_nsuccs(nsuccdim, line_buf, nodes, nnodes); // update nsuccdim from info of the way line
            continue;
        }
        if (*(line_buf) == 'r') break;                        // exit loop when we get to relations                       
    }
    
// Allocate memory for the successors of every node.
    for (i = 0; i < nnodes; i++) {
        (nodes+i)->nsucc = nsuccdim[i];
        if(((nodes+i)->successors = (unsigned long*) malloc(nsuccdim[i]*sizeof(unsigned long))) == NULL) ExitError("when allocating memory for the nodes successors", 5);
    }
    
// Allocate memory for successors index counter. Use calloc() to initialize all counters to 0.
    unsigned short* succ_counter = NULL;
    if((succ_counter = (unsigned short*) calloc(nnodes, sizeof(unsigned short))) == NULL) ExitError("when allocating memory for the nodes successor counters", 5);
    
/*** SECOND READ OF .csv FILE: Construct adjacency list ***/
    fseek(fmap, 0, SEEK_SET);                                      // rewind to the begining of file for second round of reading
    while (line_size >= 0) {
        line_size = getline(&line_buf, &line_buf_size, fmap);      // read the line
        if (*(line_buf) == '#' || *(line_buf) == 'n') continue;    // skip comment lines and nodes
        if (*(line_buf) == 'w') {                                  // process a line that corresponds to a way
            update_successors(line_buf, nodes, nnodes, succ_counter);                                                   
        }
        if (*(line_buf) == 'r') break;                              // exit loop when we get to relations
    }
    free(succ_counter);     // free nodes successors counters, not necessary anymore
    free(line_buf);         // free the allocated line buffer
    fclose(fmap);           // close .csv file
    
/*** WRITE BINARY FILE ***/
    FILE *fin;
    char name[257];
    
// Computing the total number of successors
    unsigned long ntotnsucc = 0UL;
    for(i = 0; i < nnodes; i++) ntotnsucc += nodes[i].nsucc;
    
// Setting the name of the binary file
    strcpy(name, argv[1]); strcpy(strrchr(name, '.'), ".bin");
    if ((fin = fopen (name, "wb")) == NULL)
        ExitError("the output binary data file cannot be opened", 31);
    
// Global data --- header
    if( fwrite(&nnodes, sizeof(unsigned long), 1, fin) +
        fwrite(&ntotnsucc, sizeof(unsigned long), 1, fin) != 2 )
            ExitError("when initializing the output binary data file", 32);

// Writing all nodes
    if( fwrite(nodes, sizeof(node), nnodes, fin) != nnodes )
        ExitError("when writing nodes to the output binary data file", 32);
    
// Writing sucessors in blocks
    for(i = 0; i < nnodes; i++) if(nodes[i].nsucc) {
        if ( fwrite(nodes[i].successors, sizeof(unsigned long), nodes[i].nsucc, fin) != nodes[i].nsucc )
            ExitError("when writing edges to the output binary data file", 32);
    }
    
    fclose(fin);            // close .bin file
               
/*** Free all allocated memory ***/
    for (i = 0; i < nnodes; i++) { free((nodes+i)->name); free((nodes+i)->successors); }
    free(nodes);
    free(nsuccdim);
    
    return 0;
}
