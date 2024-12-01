/*
 * cache.c
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
#include <string.h>
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = { 0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef };
    unsigned char sample_downward[16] = { 0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010 };
    int index, i = 0, j = 1, gap = 1;

    for (index = 0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;

        if (i == 0 && j == i + gap)
            j = i + (++gap);

        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k = 0; k < DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}

int check_cache_data_hit(void* addr, char type) {    // Function to find whether the cache is hit. On a cache hit, returns data read whereas on miss, returns -1.
    
    /* Fill out here */

    int byte_address = *((int*)addr);                                  // Convert the void type addr to int type and assigns it to byte_address (for calculations)
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;  // Convert byte_address to block_address by dividing by block size (8)
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;   // Take the remainder of the division of byte_address by block size (8) to create block_offset

    int cache_index = block_address % CACHE_SET_SIZE;  // Obtain cache index by taking the remainder (modulo operation) of block_address divided by set size
    int tag = block_address / CACHE_SET_SIZE;          // Obtain tag by dividing block_address by set size

    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {             // Iterate through the associativity of the cache, checking all entries in the set
        cache_entry_t* pEntry = &cache_array[cache_index][i];   // Create cache_entry_t structure pEntry from cache_array using the calculated cache index as the set index and i as the entry index
        if (pEntry->valid == 1 && pEntry->tag == tag) {         // Check if the entry's valid bit is 1 and if its tag matches the calculated tag => determines if it is a cache hit. Hit is determined if both conditions are met.

            int cache_data = -1;                                    // cache_data holds the data read from the structure (cache entry), initialized to -1
            int offset = block_offset;                              // Set the offset to decide from which byte of the cache data to retrieve, using the calculated block_offset
            switch (type) {                                         // Switch statement to determine the type of input data based on the type parameter
            case 'b':                                               // If type is 'b', i.e., byte (1 byte)
                cache_data = pEntry->data[offset];                  // Read 1 byte of data from the data array of the cache entry at the offset position
                num_bytes++;                                        // Increment num_bytes, which represents the total accessed byte count
                break;                                              // Exit the switch statement after executing case 'b'
            case 'h':                                               // If type is 'h', i.e., half-word (2 bytes)
                cache_data = *((short*)&pEntry->data[offset]);      // Read 2 bytes of data from the data array of the cache entry at the offset position by casting to short*
                num_bytes += 2;                                     // Increment num_bytes by 2
                break;                                              // Exit the switch statement after executing case 'h'
            case 'w':                                               // If type is 'w', i.e., word (4 bytes)
                cache_data = *((int*)&pEntry->data[offset]);        // Read 4 bytes of data from the data array of the cache entry at the offset position by casting to int*
                num_bytes += 4;                                     // Increment num_bytes by 4
                break;                                              // Exit the switch statement after executing case 'w'
            }

            pEntry->timestamp = global_timestamp;       // Set the timestamp of the cache entry to the global timestamp variable
            num_cache_hits++;                           // Increment the cache hit count by 1
            num_access_cycles += CACHE_ACCESS_CYCLE;    // For a cache hit, add the cache access cycle (1) to num_access_cycles
            return cache_data;                          // Return the data read from the cache
        }
    }
    num_cache_misses++;      // If cache hit does NOT occur for all entries in the set at the cache index, the cache miss count is incremented by 1

    /* Return the data */
    return -1;               // Returns -1 in case of a cache miss
}

int find_entry_index_in_set(int cache_index) {    // Function to find the entry_index of a set when the cache index is provided
    int entry_index = 0;                          // entry_index is declared and initialized to 0

    /* Check if there exists any empty cache space by checking 'valid' */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {             // Iterate through the associativity of the cache, checking all entries in the set
        cache_entry_t* pEntry = &cache_array[cache_index][i];   // Create cache_entry_t structure pEntry from cache_array using the cache index and i as the entry index
        if (pEntry->valid == 0) {                               // If the valid bit of pEntry is 0 (indicating an empty entry)
            return i;                                           // Immediately returns the index i of the entry
        }
    }

    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    int least_recent_used_timestamp = cache_array[cache_index][0].timestamp;    // If there are no empty entries, initialize least_recent_used_timestamp to the timestamp of entry 0
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {                             // Iterate through the associativity of the cache, checking all entries in the set
        cache_entry_t* pEntry = &cache_array[cache_index][i];                   // Create cache_entry_t structure pEntry from cache_array using the cache index and i as the entry index
        if (pEntry->timestamp < least_recent_used_timestamp) {                  // If the timestamp of the entry is less than least_recent_used_timestamp (indicating it was used less recently)
            least_recent_used_timestamp = pEntry->timestamp;                    // Update least_recent_used_timestamp with the timestamp of the entry
            entry_index = i;                                                    // Update entry_index with the index i of the entry
        }
    }

    return entry_index;     // Returns entry_index after the loop ends, which is the entry with the LRU timestamp
}

int access_memory(void* addr, char type) {      // For cache miss cases, this function takes the input address and data type as parameters, accesses the memory, and returns the read data.

    /* Fetch the data from the main memory and copy it to the cache */
    /* void *addr: addr is a byte address, whereas the main memory address is a word address because of 'int memory_array[]' */
    int byte_address = *((int*)addr);                                   // Convert the void type addr to int type and assigns it to byte_address (for calculations)
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;   // Convert byte_address to block_address by dividing it by the block size (8)
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;    // Calculate block_offset as the remainder of byte_address divided by the block size (8)

    int cache_index = block_address % CACHE_SET_SIZE;                   // Extract cache index by taking the remainder (modulo operation) of block_address divided by the set size

    /* Invoke find_entry_index_in_set() to copy data to the cache */
    int entry_index = find_entry_index_in_set(cache_index);             // Call find_entry_index_in_set with the calculated cache_index to find the entry_index

    cache_entry_t* pEntry = &cache_array[cache_index][entry_index];     // Create a cache_entry_t structure pEntry using cache_array with the cache index as the set index and entry_index as the entry index (cache entry to copy memory data)
    pEntry->valid = 1;                                                  // Set the valid bit of the entry to 1
    pEntry->tag = block_address / CACHE_SET_SIZE;                       // Set the tag of the entry to the result of block_address divided by the set size
    pEntry->timestamp = global_timestamp;                               // Set the timestamp of the entry to the current value of the global timestamp variable

    char* ptr = (char*)memory_array;                                    // Cast memory_array to char* type for byte (1-byte) access and store it in the char* ptr pointer variable

    int start_of_memory_address = block_address * DEFAULT_CACHE_BLOCK_SIZE_BYTE;        // Calculate the memory address (in bytes) to read by multiplying block_address by the block size (8)
    for (int i = 0; i < DEFAULT_CACHE_BLOCK_SIZE_BYTE; i++) {                           // Iterate over the block size (8)
        pEntry->data[i] = ptr[start_of_memory_address + i];                             // Copy one byte at a time from memory_array at start_of_memory_address offset by i into pEntry->data
    }

    /* Return the accessed data with a suitable type */
    int accessed_data = -1;                                 // Hold the accessed data (actually retrieved from the copied cache entry), initialized to -1
    int offset = block_offset;                              // Set the offset to determine which byte of the cache data to retrieve, using the calculated block_offset
    switch (type) {                                         // Switch statement to handle the input data type specified in the type parameter
    case 'b':                                               // If type is 'b', i.e., byte (1 byte)
        accessed_data = pEntry->data[offset];               // Read 1 byte of data from the data array of the cache entry at the offset position
        num_bytes++;                                        // Increment num_bytes, which represents the total accessed byte count
        break;                                              // Exit the switch statement after executing case 'b'
    case 'h':                                               // If type is 'h', i.e., half-word (2 bytes)
        accessed_data = *((short*)&pEntry->data[offset]);   // Read 2 bytes of data from the data array of the cache entry at the offset position by casting to short*
        num_bytes += 2;                                     // Increment num_bytes by 2
        break;                                              // Exit the switch statement after executing case 'h'
    case 'w':                                               // If type is 'w', i.e., word (4 bytes)
        accessed_data = *((int*)&pEntry->data[offset]);     // Read 4 bytes of data from the data array of the cache entry at the offset position by casting to int*
        num_bytes += 4;                                     // Increment num_bytes by 4
        break;                                              // Exit the switch statement after executing case 'w'
    }

    num_access_cycles += CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE;      // Cache miss case: Adds cache access cycle + memory access cycle (101) to num_access_cycles since both cache and memory are accessed
    return accessed_data;                                               // Returns the accessed data, which is copied from memory to the cache
}

