void AStar (node* nodes, unsigned long source, unsigned long dest, unsigned long nnodes) {
    
    AStarStatus* progress;
    if ((progress = (AStarStatus*) malloc(nnodes*sizeof(AStarStatus))) == NULL)
        ExitError("when allocating memory for the progress vector", 13);

    unsigned long i;
    for (i=0; i < nnodes; i++) {
        (progress+i)->whq = 0;                                                      // all nodes start neither in OPEN nor CLOSED list
        (progress+i)->g = INFINITY;                                                 // all nodes start 
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
    struct open_node* AUX = NULL;
    
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
        if (cur_index == dest_index) {                                                  // we have reached destination -> break
            printf("A* algorithm has reached node with ID %lu. Loop will now be exited.\n\n", cur_index);
            break;
        }
        for (succ_count = 0; succ_count < (nodes + cur_index)->nsucc; succ_count++) {   // generate successors
            succ_index = ((nodes + cur_index)->successors)[i];
            w = haversine( *(nodes + cur_index), *(nodes + succ_index) );
            successor_current_cost = (progress + cur_index)->g + w;
            if ( (progress + succ_index)->whq == 1 ) {                                  // successor is in the OPEN list
                if ( (progress + succ_index)->g <= successor_current_cost ) continue;
            }
            else if ( (progress + succ_index)->whq == 2 ) {                             // successor is in the CLOSE list
                if ( (progress + succ_index)->g <= successor_current_cost ) continue;
                /* insert successor from close to open list, maintainning ordering of f! */
            }
            else {                                                                      // successor not in OPEN nor CLOSE list
                (progress + succ_index)->h = haversine( *(nodes + succ_index), *(nodes + dest_index) );
                /* insert successor from close to open list, maintainning ordering of f! */
            }
            (progress + succ_index)->g = successor_current_cost;
            (progress + succ_index)->parent = cur_index;
        }
        (progress + cur_index)->whq = 2;                                               // move current node from OPEN to CLOSE list
        AUX = OPEN->next;                                                              // select next element in OPEN list
        free(OPEN);
        OPEN = AUX;                                                             
    }
    
    OPEN -= expanded_nodes;
    free(OPEN);
    free(progress);
}


void insert_to_OPEN (unsigned long index, AStarStatus* progress, open_node* OPEN) {
    (progress + index)->whq = 1;
    /* malloc a new node_open and compute its f = g + h from data in progress .
     * insert it in OPEN by setting pointers in ascending order of f.
     * Two distinct cases: new open_ndoe goes in between two other open_nodes
     * or new node_open goes at the end of the OPEN list. */
}


