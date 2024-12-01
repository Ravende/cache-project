/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 17, 2024
 *
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void* addr, char data_type) {                 // The retrieve_data function takes the address and data type as parameters and returns the appropriate data based on cache hit/miss status.
    int value_returned = -1; /* accessed data */                // int value_returned is declared to store the returned data value, and is initialized to -1.

    /* Invoke check_cache_data_hit() */
    value_returned = check_cache_data_hit(addr, data_type);     // Call the check_cache_data_hit function and store its return value in value_returned. If cache hit, it retrieves data from the cache; if miss, it returns -1.

    /* In case of the cache miss event, access the main memory by invoking access_memory() */
    if (value_returned == -1) {                                 // If value_returned equals -1 (cache miss)
        value_returned = access_memory(addr, data_type);        // Call access_memory to access the main memory and stores the retrieved data in value_returned.
    }

    global_timestamp++;         // Increment the global timestamp by 1 for every data access.
    return value_returned;      // Returns value_returned.
}

int main(void) {
    FILE* ifp = NULL, * ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */

    init_memory_content();
    init_cache_content();

    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    /* Fill out here by invoking retrieve_data() */
    char buffer[256];                                                   // A character array of appropriate size to temporarily store strings read from the file.
    fprintf(ofp, "[Accessed Data]\n");                                  // Write"[Accessed Data]\n" to the access_output.txt file.
    while (fgets(buffer, sizeof(buffer), ifp) != NULL) {                // Infinite loop that reads one line from access_input.txt and stores it in buffer. Terminated when reaching the end of the file (NULL).
        (void)sscanf(buffer, "%lu %c", &access_addr, &access_type);     // Extract data from the string buffer. Store the long unsigned int type data in access_addr and the char type data in access_type.

        accessed_data = retrieve_data(&access_addr, access_type);                       // Calls retrieve_data with the extracted address and data type as parameters and stores the returned data in accessed_data.
        fprintf(ofp, "%lu\t\t%c\t\t%#x\n", access_addr, access_type, accessed_data);    // Output access_addr, access_type, and accessed_data (address, data type, actual data) to access_output.txt in order (repeated for each line in the loop).
    }

    fprintf(ofp, "------------------------------------------------\n");     // Write "------------------------------------------------\n" to access_output.txt.
    switch (DEFAULT_CACHE_ASSOC) {                                          // A switch statement to determine and output the cache performance message based on the configured associativity.
    case 1:                                                                 // If associativity = 1, i.e., direct-mapped cache
        fprintf(ofp, "[Direct mapped cache performance]\n");                // Output "[Direct mapped cache performance]\n" to access_output.txt.
        break;                                                              // Exit the switch statement after executing case 1.
    case 2:                                                                 // If associativity = 2, i.e., 2-way set associative cache
        fprintf(ofp, "[2-way set associative cache performance]\n");        // Output "[2-way set associative cache performance]\n" to access_output.txt.
        break;                                                              // Exit the switch statement after executing case 2.
    case 4:                                                                 // If associativity = 4, i.e., fully associative cache
        fprintf(ofp, "[Fully associative cache performance]\n");            // Output "[Fully associative cache performance]\n" to access_output.txt.
        break;                                                              // Exit the switch statement after executing case 4.
    }
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", (float)num_cache_hits / global_timestamp, num_cache_hits, global_timestamp);   // Print the cache hit ratio calculated as num_cache_hits divided by global_timestamp to access_output.txt.
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", (float)num_bytes / num_access_cycles, num_bytes, num_access_cycles);         // Print the bandwidth calculated as num_bytes divided by num_access_cycles to access_output.txt.


    fclose(ifp);    // Close the access_input.txt file.
    fclose(ofp);    // Close the access_output.txt file.

    print_cache_entries();      // Call the print_cache_entries() function to output the cache entry details.
    return 0;                   // Return 0 at the end of the function.
}
