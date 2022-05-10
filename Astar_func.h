void AStar (node* nodes, unsigned long source, unsigned long dest, unsigned long nnodes, char* name, int evaluation, double param) {
    
    unsigned long source_index = (unsigned long)binary_search(nodes, source, 0, nnodes-1);           // find index of source in nodes vector
    unsigned long dest_index   = (unsigned long)binary_search(nodes, dest, 0, nnodes-1);             // find index of destination in nodes vector
    
    printf("A* will compute the route:\n");
    printf("\tfrom: ID %lu\tlatitude %.7f\tlongitude %.7f\n", nodes[source_index].id, nodes[source_index].lat, nodes[source_index].lon);
    printf("\tto:   ID %lu\tlatitude %.7f\tlongitude %.7f\n\n", nodes[dest_index].id, nodes[dest_index].lat, nodes[dest_index].lon);
    
// progress stores g, h, index in nodes vector and queue location for all nodes
    AStarStatus* progress = NULL;
    if ((progress = (AStarStatus*) malloc(nnodes*sizeof(AStarStatus))) == NULL) ExitError("when allocating memory for the progress vector", 19);
    unsigned long i;
    for (i = 0; i < nnodes; i++) {
        progress[i].whq = 0;                                                                  // all nodes start neither in OPEN nor CLOSED list
        progress[i].g = INFINITY;                                                             // all nodes start with INF g function
    }
    progress[source_index].whq = 1;                                                           // put source node in the OPEN list
    progress[source_index].g = 0;                                                             // g(source) = 0
    progress[source_index].h = haversine( nodes[source_index], nodes[dest_index]);            // h(source) = heuristic distance to destination

// OPEN list and auxiliary
    struct open_node* OPEN = NULL;                                                         
    if ((OPEN = (open_node*) malloc(sizeof(open_node))) == NULL) ExitError("when allocating memory for the OPEN list", 20);
    struct open_node* AUX = NULL;
// to start, only element in the OPEN list is the source node
    OPEN->index = source_index;
    OPEN->f = evaluation_function (evaluation, param, progress, nodes, source_index, source_index, dest_index);
    OPEN->next = NULL;
    unsigned long cur_index;                    // index of current node (that with minimal f) extracted from OPEN list
    unsigned short succ_count;                  // counter over the success of the node being expanded
    unsigned long succ_index;                   // index in nodes vector of the successor being processed
    double successor_current_cost;              // cost of successor (g) if we were to reach it from the current node
    double w;                                   // distance (in straight line) from current node to successor
    unsigned long expanded_nodes_counter = 0;   // counter of the number of expanded nodes
    clock_t start, end;                         // to get CPU time of running the algorithm
    double cpu_time_used;
    printf("Running A*...\n\n");
    start = clock();                            // get time before executing the A* loop
    while (OPEN != NULL) {
        cur_index = OPEN->index;
        expanded_nodes_counter += 1;
        if (cur_index == dest_index) {                                              // we have reached destination -> break
            printf("A* algorithm has reached the destination node (ID %lu).\n\n", nodes[cur_index].id);
            break;
        }
        for (succ_count = 0; succ_count < nodes[cur_index].nsucc; succ_count++) {   // generate successors
            succ_index = (nodes[cur_index].successors)[succ_count];                 // find index of generated successor
            w = haversine( nodes[cur_index], nodes[succ_index] );                   // weight of edge from current node to successor
            successor_current_cost = progress[cur_index].g + w;                     // successor cost if we were to reach successor from the current node
        /* successor is in the OPEN list */
            if ( progress[succ_index].whq == 1 ) {
                if ( progress[succ_index].g <= successor_current_cost ) continue;   // successor cost is lower than if reached from the current node: go to next successor
                else delete_from_OPEN(succ_index, progress, OPEN);                  // delete successor from OPEN list, it will be re-inserted later with updated values
            }
        /* successor is in the CLOSE list: assuming we are using a monotone heuristic, nodes in the CLOSE list are never re-expanded. */
            else if ( progress[succ_index].whq == 2 ) continue;
        /* successor not in OPEN nor CLOSE list */
            else progress[succ_index].h = haversine( nodes[succ_index], nodes[dest_index] ); // compute h function of successor
            
            progress[succ_index].g = successor_current_cost;                        // set successor cost as that coming from the current node
            progress[succ_index].parent = cur_index;                                // set successor parent as current node
            insert_to_OPEN (succ_index, progress, OPEN, nodes, evaluation, param, source_index, dest_index);
        }
        progress[cur_index].whq = 2;                                                // move current node from OPEN to CLOSE list
    // select next element in OPEN list and free memory allocated for the current node 
        AUX = OPEN->next;   free(OPEN);     OPEN = AUX;                                                                                                                      
    }
    end = clock();                                                                      // get time after ending the A* loop
    if (OPEN == NULL) ExitError("OPEN list is empty before reaching destination", 23);  // destination has not been reached

    while (OPEN != NULL) {                                                              // free the remainder of the OPEN list
        AUX = OPEN->next;   free(OPEN);     OPEN = AUX;
    }
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;                          // compute time to execute A*
    printf("The optimal distance has been found to be %.5f km.\n\n", progress[dest_index].g);
    printf("A* has expanded %lu nodes.\n\n", expanded_nodes_counter);
    printf("The A* loop has taken %.6f seconds to complete.\n\n", cpu_time_used);
       
// count the number of nodes in the path
    cur_index = dest_index;
    unsigned long path_len = 0;
    while (cur_index != source_index) {
        path_len += 1;
        cur_index = progress[cur_index].parent;
    }
    path_len += 1;
    printf("The computed path has %lu nodes.\n\n", path_len);
    
// store a vector with the indices of the nodes that make up the path
    unsigned long* path = NULL;
    if ((path = (unsigned long*) malloc(path_len*sizeof(unsigned long))) == NULL) ExitError("when allocating memory for the path vector", 24);
    path += path_len - 1;
    cur_index = dest_index;
    while (cur_index != source_index) { 
        *path = cur_index;
        cur_index = progress[cur_index].parent;
        path -= 1;
    }
    if (cur_index == source_index) *path = cur_index;
    
// check that the path makes sense: if node v follows node u, then u is in the adjacency list of v.
    bool check = is_path_correct(path, nodes);
    if (!check) ExitError("the computed path is not correct", 25);
    
// write results to a csv file
    path_to_file(nodes, path, path_len, progress, name, evaluation, param);
    
    free(path);
    free(progress);
}
