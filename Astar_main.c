#include "Astar_header.h"
#include "Astar_func.h"

int main (int argc, char *argv[]) {
    
    FILE *fin;
    if ((fin = fopen (argv[1], "rb")) == NULL)
        ExitError("the data file does not exist or cannot be opened", 11);
    
// Global data --- header    
    unsigned long nnodes;
    unsigned long ntotnsucc;
    if( fread(&nnodes, sizeof(unsigned long), 1, fin) +
        fread(&ntotnsucc, sizeof(unsigned long), 1, fin) != 2 )
            ExitError("when reading the header of the binary data file", 12);
    /*debugging*/
    printf("Total nodes: %lu\n", nnodes);
    printf("Total successors: %lu\n", ntotnsucc);
    
// Getting memory for all data
    node* nodes;
    if ((nodes = (node*) malloc(nnodes*sizeof(node))) == NULL)
        ExitError("when allocating memory for the nodes vector", 13);
    
    unsigned long* allsuccessors;
    if ((allsuccessors = (unsigned long*) malloc(ntotnsucc*sizeof(unsigned long))) == NULL)
        ExitError("when allocating memory for the edges vector", 15);
    
// Reading all data from file
    if( fread(nodes, sizeof(node), nnodes, fin) != nnodes )
        ExitError("when reading nodes from the binary data file", 17);
    if( fread(allsuccessors, sizeof(unsigned long), ntotnsucc, fin) != ntotnsucc)
        ExitError("when reading sucessors from the binary data file", 18);
    fclose(fin);
    
// Setting pointers to successors
    int i;
    for(i = 0; i < nnodes; i++) if(nodes[i].nsucc) {
        nodes[i].successors = allsuccessors;
        allsuccessors += nodes[i].nsucc;
    }
    
    unsigned long source = 771979683;   // id of nodes source and dest
    unsigned long dest = 429854583;
    
    AStar(nodes, source, dest, nnodes);
    
/*** Free all allocated memory ***/
    free(nodes);
    allsuccessors -= ntotnsucc;     // rewind allsuccessors pointer back to the beginning of the vector
    free(allsuccessors);
    
    
    return 0;
}
