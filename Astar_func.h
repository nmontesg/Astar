void AStar (node* nodes, unsigned long source, unsigned long dest, unsigned long nnodes, unsigned short dist_formula) {
    
    unsigned long source_index = (unsigned long)binary_search(nodes, source, 0, nnodes-1);           // find index of source in nodes vector
    unsigned long dest_index   = (unsigned long)binary_search(nodes, dest, 0, nnodes-1);             // find index of destination in nodes vector
    
    printf("A* will compute the route:\n");
    printf("\tfrom: ID %lu\tlatitude %.7f\tlongitude %.7f\n", (nodes+source_index)->id, (nodes+source_index)->lat, (nodes+source_index)->lon);
    printf("\tto:   ID %lu\tlatitude %.7f\tlongitude %.7f\n\n", (nodes+dest_index)->id, (nodes+dest_index)->lat, (nodes+dest_index)->lon);
    
// progress stores g, h, index in nodes vector and queue location for all nodes
    AStarStatus* progress = NULL;
    if ((progress = (AStarStatus*) malloc(nnodes*sizeof(AStarStatus))) == NULL)
        ExitError("when allocating memory for the progress vector", 13);
    unsigned long i;
    for (i = 0; i < nnodes; i++) {
        (progress+i)->whq = 0;                                                                  // all nodes start neither in OPEN nor CLOSED list
        (progress+i)->g = INFINITY;                                                             // all nodes start with INF g function
    }
    (progress + source_index)->whq = 1;                                                         // put source node in the OPEN list
    (progress + source_index)->g = 0;                                                           // g(source) = 0
    (progress + source_index)->h = distance( *(nodes + source_index), *(nodes + dest_index), dist_formula ); // h(source) = heuristic distance to destination

// OPEN list and auxiliary
    struct open_node* OPEN = NULL;                                                         
    if ((OPEN = (open_node*) malloc(sizeof(open_node))) == NULL) ExitError("when allocating memory for the OPEN list", 13);
    struct open_node* AUX = NULL;
// to start, only element in the OPEN list is the source node
    OPEN->index = source_index;
    OPEN->f = (progress + source_index)->g + (progress + source_index)->h;
    OPEN->next = NULL;
        
    unsigned long cur_index;                    // index of current node (that with minimal f) extracted from OPEN list
    unsigned short succ_count;                  // counter over the success of the node being expanded
    unsigned long succ_index;                   // index in nodes vector of the successor being processed
    double successor_current_cost;              // cost of successor (g) if we were to reach it from the current node
    double w;                                   // distance (in straight line) from current node to successor
    while (OPEN != NULL) {
        cur_index = OPEN->index;
        if (cur_index == dest_index) {                                                  // we have reached destination -> break
            printf("A* algorithm has reached node with ID %lu.\n\n", (nodes + cur_index)->id);
            break;
        }
        for (succ_count = 0; succ_count < (nodes + cur_index)->nsucc; succ_count++) {   // generate successors
            succ_index = ((nodes + cur_index)->successors)[succ_count];                 // find index of generated successor
            w = haversine( *(nodes + cur_index), *(nodes + succ_index) );               // weight of edge from current node to successor
            successor_current_cost = (progress + cur_index)->g + w;                     // successor cost if we were to reach successor from the current node
        /* successor is in the OPEN list */
            if ( (progress + succ_index)->whq == 1 ) {
                if ( (progress + succ_index)->g <= successor_current_cost ) continue;   // successor cost is lower than if reached from the current node: go to next successor
                else delete_from_OPEN(succ_index, progress, OPEN);                      // delete successor from OPEN list, it will be re-inserted later with updated values
            }
        /* successor is in the CLOSE list: assuming we are using a monotone heuristic, nodes in the CLOSE list are never re-expanded. */
            else if ( (progress + succ_index)->whq == 2 ) continue;
        /* successor not in OPEN nor CLOSE list */
            else (progress + succ_index)->h = distance( *(nodes + succ_index), *(nodes + dest_index), dist_formula ); // compute h function of successor
            
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
    
    printf("The optimal distance has been found to be: %.7f km\n\n", (progress + dest_index)->g);
       
// count the number of nodes in the path
    cur_index = dest_index;
    unsigned long path_len = 0;
    while (cur_index != source_index) {
        path_len += 1;
        cur_index = (progress + cur_index)->parent;
    }
    path_len += 1;
    printf("The computed path has %lu nodes.\n\n", path_len);
    
// store a vector with the indices of the nodes that make up the path
    unsigned long* path = NULL;
    if ((path = (unsigned long*) malloc(path_len*sizeof(unsigned long))) == NULL) ExitError("when allocating memory for the path vector", 13);
    path += path_len - 1;
    cur_index = dest_index;
    while (cur_index != source_index) { 
        *path = cur_index;
        cur_index = (progress + cur_index)->parent;
        path -= 1;
    }
    if (cur_index == source_index) *path = cur_index;
    
    /*debugging: print path*/
    i = 0;
    printf("%lu\n\n", source_index);
    for (i=0; i<path_len; i++) {
        printf("%lu\n", path[i]);
    }
    printf("\n%lu\n\n", dest_index);
    /*int j = 0;
    for (i=0; i<path_len; i++) {
        printf("%lu %d --> ", path[i], (nodes+path[i])->nsucc);
        for (j=0; j < (nodes+path[i])->nsucc; j++) printf("%lu\t", ((nodes+path[i])->successors)[j]);
        printf("\n");
    }*/
    
    /*unsigned long* current = path;
    unsigned long* next = current + 1;
    while (next != NULL) {
        printf("%lu %lu\n", *current, *next);
        current = next;
        next = current + 1;
    }*/

// check that the path makes sense: if node v follows node u, then u is in the adjacency list of v.
    bool check = is_path_correct(path, nodes);
    if (check) ExitError("the computed path is not correct", 13);
    
    free(path);
    free(progress);
}
