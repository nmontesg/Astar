#include "Astar_header.h"
#include "heuristics.h"
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
    unsigned long i;
    for(i = 0; i < nnodes; i++) if(nodes[i].nsucc) {
        nodes[i].successors = allsuccessors;
        allsuccessors += nodes[i].nsucc;
    }

// User chooses IDs of source and destination nodes:
    unsigned long source;             // id of source
    unsigned long dest;               // id of dest
    printf("Choose IDs of starting and destination nodes:\n");
// suggestions if we are working with map of Spain
    if (*argv[1] == 's') {            
        printf("\tSanta Maria del Mar, Barcelona : 240949599\n");
        printf("\tGiralda, Sevilla : 195977239\n");
    }
// suggestions if we are working with map of Catalonia
    if (*argv[1] == 'c') {            
        printf("\tSomewhere in Girona : 771979683\n");
        printf("\tSomewhere in Lleida : 429854583\n");
    }
// input ID of starting node
    printf("ID of starting node: ");  
    if (scanf("%lu", &source) != 1) ExitError("when reading the id of starting node", 18);
    while ( binary_search(nodes, source, 0, nnodes-1) == -1 ) {
        printf("Invalid choice of starting node. Please enter starting node id again: ");
        if (scanf("%lu", &source) != 1) ExitError("when reading the id of starting node", 18);
    }
// input ID of destination node
    printf("ID of destination node: ");
    if (scanf("%lu", &dest) != 1) ExitError("when reading the id of destination node", 18);
    while ( binary_search(nodes, dest, 0, nnodes-1) == -1 ) {
        printf("Invalid choice of destination node. Please enter destination node id again: ");
        if (scanf("%lu", &dest) != 1) ExitError("when reading the id of destination node", 18);
    }
    printf("\n");
    
// User chooses distance formula
    unsigned short dist_func;
    printf("Choose the distance formula (for more info go to http://movable-type.co.uk/scripts/latlong.html):\n");
    printf("\t1 : Haversine formula\n");
    printf("\t2 : Spherical Law of Cosines\n");
    printf("\t3 : Equirectangular approximation\n");
    if (scanf("%hu", &dist_func) != 1) ExitError("when reading the choice of distance formula", 18);
    while ( dist_func<1 || dist_func>3 ) {
        printf("Invalid choice of distance formula. Please enter your choice again: ");
        if (scanf("%hu", &dist_func) != 1) ExitError("when reading the choice of distance formula", 18);
    }
    printf("\n");
    
    AStar(nodes, source, dest, nnodes, dist_func);
    
/*** Free all allocated memory ***/
    free(nodes);
    allsuccessors -= ntotnsucc;     // rewind allsuccessors pointer back to the beginning of the vector
    free(allsuccessors);
        
    return 0;
}
