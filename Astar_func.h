void AStar (node* nodes, unsigned long source, unsigned long dest, unsigned long nnodes) {
    
    AStarStatus* progress;
    if ((progress = (AStarStatus*) malloc(nnodes*sizeof(AStarStatus))) == NULL)
        ExitError("when allocating memory for the progress vector", 13);

    unsigned long i;
    for (i=0; i < nnodes; i++) {
        (progress+i)->whq = 0;                                                      // all nodes start neither in OPEN nor CLOSED list
        (progress+i)->g = INFINITY;                                                 // all nodes start with INF g function
    }
        
    unsigned long source_index = binary_search(nodes, source, 0, nnodes);           // find index of source in nodes vector
    unsigned long dest_index = binary_search(nodes, dest, 0, nnodes);               // find index of destination in nodes vector
    
    (progress + source_index)->whq = 1;                                             // put source in OPEN list
    (progress + source_index)->g = 0;
    (progress + source_index)->h = haversine( *(nodes + source_index), *(nodes + dest_index) );

// OPEN list
    struct open_node* OPEN;                                                         
    if ((OPEN = (open_node*) malloc(sizeof(open_node))) == NULL)
        ExitError("when allocating memory for the OPEN list", 13);
    
// to start, only element in the OPEN list is the source node
    OPEN->f = (progress + source_index)->g + (progress + source_index)->h;
    OPEN->index = source_index;
    OPEN->next = NULL;
    
    unsigned long cur_index;
    unsigned short succ_count;
    unsigned long succ_index;
    double successor_current_cost;
    double w;
    unsigned long expanded_nodes = 0;
    while (OPEN != NULL) {
        expanded_nodes += 0;
        cur_index = OPEN->index;
        /*debugging*/printf("Expanding node: %lu (id %lu, nsucc %d)\n", cur_index, (nodes+cur_index)->id, (nodes+cur_index)->nsucc);
        if (cur_index == dest_index) {                                                  // we have reached destination -> break
            printf("A* algorithm has reached node with ID %lu. Loop will now be exited.\n\n", cur_index);
            break;
        }
        for (succ_count = 0; succ_count < (nodes + cur_index)->nsucc; succ_count++) {   // generate successors
            succ_index = ((nodes + cur_index)->successors)[i];
            /*debugging*/printf("\tExploring successor: %lu (id %lu)\n", succ_index, (nodes+succ_index)->id);
            w = haversine( *(nodes + cur_index), *(nodes + succ_index) );
        }
        (progress + cur_index)->whq = 2;                                                // move current node from OPEN to CLOSE list
        OPEN = OPEN->next;                                                              // select next element in OPEN list                                                            
    }
    
    free(progress);
}





