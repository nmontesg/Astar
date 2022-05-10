#include "Astar_header.h"
#include "Astar_func.h"

int main (int argc, char *argv[]) {
    
    FILE *fin;
    if ((fin = fopen (argv[1], "rb")) == NULL)
        ExitError("the data file does not exist or cannot be opened", 1);
    
// Global data --- header    
    unsigned long nnodes;
    unsigned long ntotnsucc;
    unsigned long totnamelen;
    if( fread(&nnodes, sizeof(unsigned long), 1, fin) +
        fread(&ntotnsucc, sizeof(unsigned long), 1, fin) +
        fread(&totnamelen, sizeof(unsigned long), 1, fin) != 3 )
            ExitError("when reading the header of the binary data file", 2);
    
// Getting memory for all data
    node* nodes;
    if ((nodes = (node*) malloc(nnodes*sizeof(node))) == NULL)
        ExitError("when allocating memory for the nodes vector", 3);
    
    unsigned long* allsuccessors;
    if ((allsuccessors = (unsigned long*) malloc(ntotnsucc*sizeof(unsigned long))) == NULL)
        ExitError("when allocating memory for the edges vector", 4);
    
    char* allnames = NULL;
    if((allnames = (char*) malloc((totnamelen)*sizeof(char))) == NULL) ExitError("when allocating memory for allnames vector", 5);
    char* allnames_aux = allnames; 
    
// Reading all data from file
    if( fread(nodes, sizeof(node), nnodes, fin) != nnodes )
        ExitError("when reading nodes from the binary data file", 6);
    
    if( fread(allsuccessors, sizeof(unsigned long), ntotnsucc, fin) != ntotnsucc)
        ExitError("when reading sucessors from the binary data file", 7);

    if( fread(allnames, sizeof(char), totnamelen, fin) != totnamelen)
        ExitError("when reading names from the binary data file", 8);
    
    fclose(fin);
    
// Setting pointers to successors and copying the names of all the nodes
    unsigned long i;
    for (i = 0; i < nnodes; i++) {
        if(nodes[i].nsucc) {
            nodes[i].successors = allsuccessors;
            allsuccessors += nodes[i].nsucc;
        }
        if ((nodes[i].name = (char*) malloc(nodes[i].namelen + 1)) == NULL) ExitError("when allocating memory for a node name", 9);
        strncpy(nodes[i].name, allnames, nodes[i].namelen);
        memset(nodes[i].name + nodes[i].namelen, '\0', 1);
        allnames += nodes[i].namelen;
    }
    free(allnames_aux);
    
// User chooses IDs of source and destination nodes:
    unsigned long source;             // id of source
    unsigned long dest;               // id of dest
    printf("\nChoose IDs of starting and destination nodes:\n");
// suggestions if we are working with map of Spain
    if (*argv[1] == 's') {  
        printf("\tSuggestions:\n");
        printf("\t\tSanta Maria del Mar, Barcelona : 240949599\n");
        printf("\t\tGiralda, Sevilla : 195977239\n\n");
    }
// suggestions if we are working with map of Catalonia
    else if (*argv[1] == 'c') {   
        printf("\tSuggestions:\n");
        printf("\t\tSomewhere in Girona : 771979683\n");
        printf("\t\tSomewhere in Lleida : 429854583\n\n");
    }
// input ID of starting node
    printf("ID of starting node: ");  
    if (scanf("%lu", &source) != 1) ExitError("when reading the id of starting node", 10);
    while ( binary_search(nodes, source, 0, nnodes-1) == -1 ) {
        printf("Invalid choice of starting node. Please enter starting node ID again: ");
        if (scanf("%lu", &source) != 1) ExitError("when reading the ID of starting node", 11);
    }
// input ID of destination node
    printf("ID of destination node: ");
    if (scanf("%lu", &dest) != 1) ExitError("when reading the id of destination node", 12);
    while ( binary_search(nodes, dest, 0, nnodes-1) == -1 ) {
        printf("Invalid choice of destination node. Please enter destination node ID again: ");
        if (scanf("%lu", &dest) != 1) ExitError("when reading the ID of destination node", 13);
    }
    printf("\n");
// choose evaluation function mode
    printf("Choose evaluation function:\n");
    printf("\t1 - Default:              f = g + h\n");
    printf("\t2 - Weighted:             f = (1-w)·g + w·h\n");
    printf("\t3 - Dynamic weighting:    f = g + h + e·(1-d/N)·h\n");
    int evaluation = 0;
    if (scanf("%d", &evaluation) != 1) ExitError("when reading the evaluation function", 14);
    while ( evaluation < 1 || evaluation > 3) {
        printf("Invalid choice of evaluation function. Please enter your choice of evaluation again: ");
        if (scanf("%d", &evaluation) != 1) ExitError("when reading the evaluation function", 15);
    }
    double param = 0;
    if (evaluation == 1) printf("You have chosen default evaluation.\n");
    else if (evaluation == 2) {
        printf("You have chosen weighted evaluation. Please input parameter w: ");
        if (scanf("%lf", &param) != 1) ExitError("when reading the parameter for weighted evaluation", 16);
        while ( param < 0 || param > 1) {
            printf("Invalid choice of w. Please enter w again: ");
            if (scanf("%lf", &param) != 1) ExitError("when reading the parameter for weighted evaluation", 17);
        }
    }
    else if (evaluation == 3) {
        printf("You have chosen dynamic weighting evaluation. Please input parameter e (epsilon): ");
        if (scanf("%lf", &param) != 1) ExitError("when reading the parameter for weighted evaluation", 18);
    }
    printf("\n");
    
    AStar(nodes, source, dest, nnodes, argv[1], evaluation, param);
    
/*** Free all allocated memory ***/
    for (i = 0; i < nnodes; i++) free(nodes[i].name);
    free(nodes);
    allsuccessors -= ntotnsucc;     // rewind allsuccessors pointer back to the beginning of the vector
    free(allsuccessors);
            
    return 0;
}
