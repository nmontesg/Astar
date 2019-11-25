void AStar (node* nodes, unsigned long source, unsigned long dest, unsigned long nnodes) {
    
    unsigned long source_index = (unsigned long)binary_search(nodes, source, 0, nnodes-1);           // find index of source in nodes vector
    unsigned long dest_index   = (unsigned long)binary_search(nodes, dest, 0, nnodes-1);             // find index of destination in nodes vector
    
    printf("A* will compute the route:\n");
    printf("\tfrom: ID %lu\tlatitude %.7f\tlongitude %.7f\n", (nodes+source_index)->id, (nodes+source_index)->lat, (nodes+source_index)->lon);
    printf("\tto:   ID %lu\tlatitude %.7f\tlongitude %.7f\n", (nodes+dest_index)->id, (nodes+dest_index)->lat, (nodes+dest_index)->lon);
    
    AStarStatus* progress = NULL;
    if ((progress = (AStarStatus*) malloc(nnodes*sizeof(AStarStatus))) == NULL)
        ExitError("when allocating memory for the progress vector", 13);

    unsigned long i;
    for (i=0; i < nnodes; i++) {
        (progress+i)->whq = 0;                                                      // all nodes start neither in OPEN nor CLOSED list
        (progress+i)->g = INFINITY;                                                 // all nodes start with INF g function
    }
    
    (progress + source_index)->whq = 1;                                             // put source in OPEN list
    (progress + source_index)->g = 0;
    (progress + source_index)->h = haversine( *(nodes + source_index), *(nodes + dest_index) );

// OPEN list and auxiliary
    struct open_node* OPEN = NULL;                                                         
    if ((OPEN = (open_node*) malloc(sizeof(open_node))) == NULL) ExitError("when allocating memory for the OPEN list", 13);
    struct open_node* AUX = NULL;
    
// to start, only element in the OPEN list is the source node
    OPEN->index = source_index;
    OPEN->f = (progress + source_index)->g + (progress + source_index)->h;
    OPEN->next = NULL;
        
    unsigned long cur_index;
    unsigned short succ_count;
    unsigned long succ_index;
    double successor_current_cost;
    double w;
    while (OPEN != NULL) {
        cur_index = OPEN->index;
        /*debugging*///printf("Expanding node: %lu (id %lu, nsucc %d)\n", cur_index, (nodes+cur_index)->id, (nodes+cur_index)->nsucc);
        if (cur_index == dest_index) {                                                  // we have reached destination -> break
            printf("A* algorithm has reached node with ID %lu. Loop will now be exited.\n\n", (nodes + cur_index)->id);
            break;
        }
        for (succ_count = 0; succ_count < (nodes + cur_index)->nsucc; succ_count++) {   // generate successors
            succ_index = ((nodes + cur_index)->successors)[succ_count];                 // find index of generated successor
            /*debugging*///printf("\tExploring successor: %lu (id %lu)\n", succ_index, (nodes+succ_index)->id);
            w = haversine( *(nodes + cur_index), *(nodes + succ_index) );               // weight of edge from current node to successor
            successor_current_cost = (progress + cur_index)->g + w;                     // successor cost if we were to reach successor from the current node
        // successor is in the OPEN list
            if ( (progress + succ_index)->whq == 1 ) {   
                /*debugging*///printf("\tsuccessor in OPEN list\n");
                if ( (progress + succ_index)->g <= successor_current_cost ) {           // successor cost is lower than if reached from the current node: go to next successor
                    /*debugging*///printf("\tsuccessor stays in OPEN list\n\n");
                    continue;}   // successor lower g than if current node was its parent: go to next successor
                else {
                    /*delete successor from OPEN list, it will be re-inserted later with updated values*/
                }
            }
        // successor is in the CLOSE list
            else if ( (progress + succ_index)->whq == 2 ) { 
                /*debugging*///printf("\tsuccessor in CLOSE list\n");
                if ( (progress + succ_index)->g <= successor_current_cost ) {
                    /*debugging*///printf("\tsuccessor stays in CLOSE list\n\n");
                    continue;}   // successor lower g than if current node was its parent: go to next successor
            }
        // successor not in OPEN nor CLOSE list
            else {                                                                      
                /*debugging*///printf("\tsuccessor not in OPEN nor CLOSE list\n");
                (progress + succ_index)->h = haversine( *(nodes + succ_index), *(nodes + dest_index) ); // compute h function of successor
            }
            (progress + succ_index)->g = successor_current_cost;                        // set successor cost as that coming from the current node
            (progress + succ_index)->parent = cur_index;                                // set successor parent as current node
            insert_to_OPEN(succ_index, progress, OPEN);                                 // insert successor from close to open list, maintaining ordering of f
        }
        (progress + cur_index)->whq = 2;                                                // move current node from OPEN to CLOSE list
    // select next element in OPEN list and free memory allocated for the current node 
        AUX = OPEN->next;   free(OPEN);     OPEN = AUX;                                                                                                                      
    }
    
    if (OPEN == NULL) ExitError("OPEN list is empty before reaching destination", 13);  // destination has not been reached

    while (OPEN != NULL) {                                                              // free the remainder of the OPEN list
        AUX = OPEN->next;   free(OPEN);     OPEN = AUX;
    }
    
    printf("total distance: %f km\n", (progress + dest_index)->g);
    
    free(progress);
}





