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
    unsigned char sample_upward[16] = {0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef};
    unsigned char sample_downward[16] = {0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010};
    int index, i=0, j=1, gap = 1;
    
    for (index=0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;
        
        if (i == 0 && j == i+gap)
            j = i + (++gap);
            
        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}  

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
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
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}

int check_cache_data_hit(void *addr, char type) {

    /* Fill out here */

    //0. addr 변환해서 block address로
    int byte_address = *((int*)addr);
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    //int blocks_in_cache = DEFAULT_CACHE_SIZE_BYTE / DEFAULT_CACHE_BLOCK_SIZE_BYTE;  // 4

    //0-1. block address에서 cacheIndex/tag 추출
    int cache_index = block_address % CACHE_SET_SIZE;
    int tag = block_address / CACHE_SET_SIZE;

    //1. cache index 찾아야 하고
    //2. index의 valid bit 1인지 확인
    //3. 그 tag 일치하는지
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t* pEntry = &cache_array[cache_index][i];
        if (pEntry->valid == 1 && pEntry->tag == tag) {

            int cache_data = -1;
            int offset = block_offset;  // 확인 필요!
            printf("Address: %d\tOffset: %d\n", byte_address, offset);
            switch (type) {
            case 'b':
                cache_data = pEntry->data[offset];
                num_bytes++;
                break;
            case 'h':
                cache_data = *((short*)&pEntry->data[offset]);
                num_bytes += 2;
                break;
            case 'w':
                cache_data = *((int*)&pEntry->data[offset]);
                num_bytes += 4;
                break;
            }

            pEntry->timestamp = global_timestamp;
            num_cache_hits++;
            num_access_cycles += CACHE_ACCESS_CYCLE;
            return cache_data;
        }
    }
    num_cache_misses++;

    /* Return the data */
    return -1;
}

int find_entry_index_in_set(int cache_index) {
    int entry_index = 0;

    /* Check if there exists any empty cache space by checking 'valid' */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t* pEntry = &cache_array[cache_index][i];
        if (pEntry->valid == 0) {
            return i;
        }
    }

    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    int least_recent_used_timestamp = cache_array[cache_index][0].timestamp;
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t* pEntry = &cache_array[cache_index][i];
        if (pEntry->timestamp < least_recent_used_timestamp) {
            least_recent_used_timestamp = pEntry->timestamp;
            entry_index = i;
        }
    }

    return entry_index; 
}

int access_memory(void* addr, char type) {

    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */

    int byte_address = *((int*)addr);
    int word_address = byte_address / WORD_SIZE_BYTE;
    int word_offset = byte_address % WORD_SIZE_BYTE;

    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;

    //int blocks_in_cache = DEFAULT_CACHE_SIZE_BYTE / DEFAULT_CACHE_BLOCK_SIZE_BYTE;  // 4
    int cache_index = block_address % CACHE_SET_SIZE;

    /* You need to invoke find_entry_index_in_set() for copying to the cache */
    int entry_index = find_entry_index_in_set(cache_index);

    cache_entry_t* pEntry = &cache_array[cache_index][entry_index];
    pEntry->valid = 1;
    pEntry->tag = block_address / CACHE_SET_SIZE;
    pEntry->timestamp = global_timestamp;

    char* ptr = (char*)memory_array;

    int start_of_memory_address = block_address * DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    for (int i = 0; i < DEFAULT_CACHE_BLOCK_SIZE_BYTE; i++) {
        printf("Word Address: %d\tOffset: %d\n", word_address, start_of_memory_address + i);
        pEntry->data[i] = ptr[start_of_memory_address + i];
    }

    /* Return the accessed data with a suitable type */
    int accessed_data = -1;
    int offset = block_offset;  // 확인 필요!
    printf("Address: %d\tOffset: %d\n", byte_address, offset);
    switch (type) {
    case 'b':
        accessed_data = pEntry->data[offset];
        num_bytes++;
        break;
    case 'h':
        accessed_data = *((short*)&pEntry->data[offset]);
        num_bytes += 2;
        break;
    case 'w':
        accessed_data = *((int*)&pEntry->data[offset]);
        num_bytes += 4;
        break;
    }
    printf("\n");

    num_access_cycles += CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE;
    return accessed_data;
}
