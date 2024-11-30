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

int retrieve_data(void *addr, char data_type) {
    int value_returned = 0; /* accessed data */

    int address = *((int*)addr);
    int block_address = address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int blocks_in_cache = DEFAULT_CACHE_SIZE_BYTE / DEFAULT_CACHE_BLOCK_SIZE_BYTE;  // 4
    int cache_index = block_address % (blocks_in_cache / DEFAULT_CACHE_ASSOC);
    int tag = block_address / (blocks_in_cache / DEFAULT_CACHE_ASSOC);

    extern cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
    printf("Block Address: %d, Cache Index: %d, Tag: %d\n", block_address, cache_index, tag);

    /* Invoke check_cache_data_hit() */
    int hit_check = check_cache_data_hit(addr, data_type);
    printf("Hit: %d\n", hit_check);

    if (hit_check) {

        if (cache_index >= 0 && cache_index < CACHE_SET_SIZE) {
            for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
                cache_entry_t* pEntry = &cache_array[cache_index][i];
                printf("Valid: %d\n", pEntry->valid);

                if (pEntry->valid == 1 && pEntry->tag == tag) {

                    switch (data_type) {
                        printf("%d\n", address);
                    case 'b':
                        value_returned = pEntry->data[address % WORD_SIZE_BYTE];
                        num_bytes++;
                        break;
                    case 'h':
                        value_returned = *((short*)&pEntry->data[address % WORD_SIZE_BYTE]);
                        num_bytes += 2;
                        printf("Cache Hit@ Entry Index: %d\t%c type - Accessed Data: %d\n", i, data_type, address % WORD_SIZE_BYTE);
                        break;
                    case 'w':
                        value_returned = *((int*)&pEntry->data[address % WORD_SIZE_BYTE]);
                        num_bytes += 4;
                        break;
                    }
                }
            }
        }
        num_access_cycles += CACHE_ACCESS_CYCLE;
    } else {
        value_returned = access_memory(addr, data_type);
        num_access_cycles += CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE;
    }

    global_timestamp++;

    return value_returned;
}

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 
    
    init_memory_content();
    init_cache_content();
    
    ifp = fopen("C:/Users/raven/OneDrive/πŸ≈¡ »≠∏È/2-2/Computer Architecture/20493-01-Project-Skeleton/access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    ofp = fopen("C:/Users/raven/OneDrive/πŸ≈¡ »≠∏È/2-2/Computer Architecture/20493-01-Project-Skeleton/access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    /* Fill out here by invoking retrieve_data() */

    char buffer[256];
    fprintf(ofp, "[Accessed Data]\n");
    while (fgets(buffer, sizeof(buffer), ifp) != NULL) {
        //printf("Buffer content: %s\n", buffer);

        (void)sscanf(buffer, "%lu %c", &access_addr, &access_type);
        
        accessed_data = retrieve_data(&access_addr, access_type);
        fprintf(ofp, "%lu\t\t%c\t\t%#x\n", access_addr, access_type, accessed_data);
    }

    fprintf(ofp, "------------------------------------------------\n");
    switch (DEFAULT_CACHE_ASSOC) {
    case 1:
        fprintf(ofp, "[Direct mapped cache performance]\n");
        break;
    case 2:
        fprintf(ofp, "[2-way set associative cache performance]\n");
        break;
    case 4: 
        fprintf(ofp, "[Fully associative cache performance]\n");
        break;
    }
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", (float)num_cache_hits/global_timestamp, num_cache_hits, global_timestamp);
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", (float)num_bytes / num_access_cycles, num_bytes, num_access_cycles);
    

    fclose(ifp);
    fclose(ofp);
    
    print_cache_entries();
    return 0;
}
