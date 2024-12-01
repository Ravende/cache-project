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

int check_cache_data_hit(void *addr, char type) {    // cache의 hit 여부를 찾는 함수. cache hit 시 cache에서 바로 읽은 데이터를, miss 시 -1을 반환한다.

    /* Fill out here */

    int byte_address = *((int*)addr);                                  // void 타입의 addr을 int 타입으로 변환해 byte_address에 넣음 (연산 위함)
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;  // byte_address를 block size(8)로 나눠 block_address로 변환함
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;   // byte_address를 blcok size(8)로 나누는 연산의 나머지를 취해 block_offset 만듦

    int cache_index = block_address % CACHE_SET_SIZE;  // block_address를 set size로 나눈 나머지를 취해 (modulo 연산) cache index 추출
    int tag = block_address / CACHE_SET_SIZE;          // block_address를 set size로 나눈 몫을 취해 tag 추출

    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {             // i를 캐시의 associativity만큼 돌면서 해당 set의 entry 하나씩 전부 확인
        cache_entry_t* pEntry = &cache_array[cache_index][i];   // cache_array 배열에 계산한 cache index를 set index로, i를 entry index로 주어서 cache_entry_t 구조체 pEntry 생성
        if (pEntry->valid == 1 && pEntry->tag == tag) {         // 해당 entry의 valid bit이 1인지, tag가 위에서 계산한 tag와 일치하는지 함께 확인 => cache hit인지 확인. 둘 다 충족될 때 hit 판정

            int cache_data = -1;                                    // 구조체(캐시 엔트리)에서 가져오는 데이터를 담은 cache_data (초기값 -1)
            int offset = block_offset;                              // 캐시의 데이터를 어느 바이트부터 가져올지 결정하기 위한 offset: 위에서 계산한 block_offset으로 설정
            switch (type) {                                         // 함수 파라미터로 전달받은 type(input의 데이터 타입)에 따라 판단하는 switch문
            case 'b':                                               // type이 'b', 즉 바이트(1byte)인 경우
                cache_data = pEntry->data[offset];                  // 캐시 엔트리의 data 배열의 offset 위치부터 데이터를 1바이트 읽어옴
                num_bytes++;                                        // 지금까지 접근한 바이트 수를 뜻하는 num_bytes에 +1
                break;                                              // case 'b' 실행 후 switch문 밖으로 탈출
            case 'h':                                               // type이 'h', 즉 하프워드(2byte)인 경우
                cache_data = *((short*)&pEntry->data[offset]);      // short* 타입으로 주소 변환. => 캐시 엔트리의 data 배열의 offset 위치부터 데이터를 short 크기만큼(2바이트) 읽어옴
                num_bytes += 2;                                     // 지금까지 접근한 바이트 수를 뜻하는 num_bytes에 +2
                break;                                              // case 'h' 실행 후 switch문 밖으로 탈출
            case 'w':                                               // type이 'w', 즉 워드(4byte)인 경우
                cache_data = *((int*)&pEntry->data[offset]);        // int* 타입으로 주소 변환. => 캐시 엔트리의 data 배열의 offset 위치부터 데이터를 int 크기만큼(4바이트) 읽어옴
                num_bytes += 4;                                     // 지금까지 접근한 바이트 수를 뜻하는 num_bytes에 +4
                break;                                              // case 'w' 실행 후 switch문 밖으로 탈출
            }

            pEntry->timestamp = global_timestamp;       // 해당 캐시 엔트리의 timestamp를 지금까지의 전역 timestamp 변수 값으로 설정
            num_cache_hits++;                           // cache hit 수 +1
            num_access_cycles += CACHE_ACCESS_CYCLE;    // cache hit이므로 access cycles 수에 cache access cycle인 +1
            return cache_data;                          // 캐시로부터 읽어온 데이터 반환
        }
    }
    num_cache_misses++;      // 해당 cache index(set index)의 모든 entry에서 cache hit이 되지 못했을 경우 cache miss. cache miss 수 +1

    /* Return the data */
    return -1;               // cache miss의 경우 -1 리턴
}

int find_entry_index_in_set(int cache_index) {    // cache index가 주어지면 해당 set의 entry_index를 찾는 함수
    int entry_index = 0;                          // entry_index 선언. 초기값 0으로 설정

    /* Check if there exists any empty cache space by checking 'valid' */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {             // i를 캐시의 associativity만큼 돌면서 해당 set의 entry 하나씩 전부 확인
        cache_entry_t* pEntry = &cache_array[cache_index][i];   // cache_array 배열에 파라미터로 받아온 cache index를 set index로, i를 entry index로 주어서 cache_entry_t 구조체 pEntry를 생성
        if (pEntry->valid == 0) {                               // 만약 pEntry의 valid bit이 0이면 (= 빈 엔트리가 있으면)
            return i;                                           // 해당 entry의 인덱스 i를 바로 리턴
        }
    }

    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    int least_recent_used_timestamp = cache_array[cache_index][0].timestamp;    // 빈 entry가 없는 경우. 사용한 지 가장 오래된 엔트리의 timestamp 저장하는 변수 - 초기값으로 entry 0의 timestamp 저장
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {                             // i를 캐시의 associativity만큼 돌면서 해당 set의 entry 하나씩 전부 확인
        cache_entry_t* pEntry = &cache_array[cache_index][i];                   // cache_array 배열에 파라미터로 받아온 cache index를 set index로, i를 entry index로 주어서 cache_entry_t 구조체 pEntry를 생성
        if (pEntry->timestamp < least_recent_used_timestamp) {                  // 해당 entry의 timestamp(어느 시점에 사용되었는지 판별)가 least_recent_used_timestamp보다 작으면 (= 더 오래되었으면)
            least_recent_used_timestamp = pEntry->timestamp;                    // least_recent_used_timestamp 변수에 해당 entry의 timestamp를 저장하고
            entry_index = i;                                                    // entry_index에는 해당 entry의 인덱스 i를 저장한다.
        }
    }

    return entry_index;     // for문이 끝나고 LRU timestamp의 entry_index가 나옴. entry_index 반환한다
}

int access_memory(void* addr, char type) {      // (cache miss의 경우,) input의 주소와 데이터 타입을 파라미터로 받아 메모리에 접근하고 읽은 데이터를 반환해주는 함수

    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */
    int byte_address = *((int*)addr);                                   // void 타입의 addr을 int 타입으로 변환해 byte_address에 넣음 (연산 위함)
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;   // byte_address를 block size(8)로 나눠 block_address로 변환함
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;    // byte_address를 blcok size(8)로 나누는 연산의 나머지를 취해 block_offset 만듦

    int cache_index = block_address % CACHE_SET_SIZE;                   // block_address를 set size로 나눈 나머지를 취해 (modulo 연산) cache index 추출

    /* You need to invoke find_entry_index_in_set() for copying to the cache */
    int entry_index = find_entry_index_in_set(cache_index);             // 계산한 cache_index를 파라미터로 넣은 find_entry_index_in_set 함수를 불러 entry_index를 찾음

    cache_entry_t* pEntry = &cache_array[cache_index][entry_index];     // cache_array 배열에 계산한 cache index를 set 인덱스로, 파라미터로 받아온 entry_index를 entry 인덱스로 주어서 cache_entry_t 구조체 pEntry를 생성 (메모리의 데이터 copy할 캐시 엔트리)
    pEntry->valid = 1;                                                  // 해당 entry의 valid bit을 1로 설정
    pEntry->tag = block_address / CACHE_SET_SIZE;                       // 해당 entry의 tag를 block_address를 set size로 나눈 값으로 설정
    pEntry->timestamp = global_timestamp;                               // 해당 entry의 timestamp를 지금까지의 전역 timestamp 변수 값으로 설정

    char* ptr = (char*)memory_array;                                    // memory_array를 char(1바이트) 단위로 접근하기 위해 char* 타입으로 캐스팅하여 char* ptr 포인터 변수에 저장

    int start_of_memory_address = block_address * DEFAULT_CACHE_BLOCK_SIZE_BYTE;        // 읽어올 메모리의 주소(바이트 단위)를 결정하기 위해 block address에 block size(8)을 곱해서 계산
    for (int i = 0; i < DEFAULT_CACHE_BLOCK_SIZE_BYTE; i++) {                           // for문을 block size(8) 크기만큼 돌면서
        pEntry->data[i] = ptr[start_of_memory_address + i];                             // pEntry의 data의 i주소(0~7)에 memory_array의 start_of_memory_address 위치부터 i씩 더해주며 한 바이트씩, 모두 2워드 저장
    }

    /* Return the accessed data with a suitable type */
    int accessed_data = -1;                                 // 메모리에서 접근한 (실제로는 해당 메모리의 데이터를 복사한 캐시 엔트리에서 가져오는) 데이터를 담은 accessed_data (초기값 -1)
    int offset = block_offset;                              // 캐시의 데이터를 어느 바이트부터 가져올지 결정하기 위한 offset: 위에서 계산한 block_offset으로 설정
    switch (type) {                                         // 함수 파라미터로 전달받은 type(input의 데이터 타입)에 따라 판단하는 switch문
    case 'b':                                               // type이 'b', 즉 바이트(1byte)인 경우
        accessed_data = pEntry->data[offset];               // 캐시 엔트리의 data 배열의 offset 위치부터 데이터를 1바이트 읽어옴
        num_bytes++;                                        // 지금까지 접근한 바이트 수를 뜻하는 num_bytes에 +1
        break;                                              // case 'b' 실행 후 switch문 밖으로 탈출
    case 'h':                                               // type이 'h', 즉 하프워드(2byte)인 경우
        accessed_data = *((short*)&pEntry->data[offset]);   // short* 타입으로 주소 변환. => 캐시 엔트리의 data 배열의 offset 위치부터 데이터를 short 크기만큼(2바이트) 읽어옴
        num_bytes += 2;                                     // 지금까지 접근한 바이트 수를 뜻하는 num_bytes에 +2
        break;                                              // case 'h' 실행 후 switch문 밖으로 탈출
    case 'w':                                               // type이 'w', 즉 워드(4byte)인 경우
        accessed_data = *((int*)&pEntry->data[offset]);     // int* 타입으로 주소 변환. => 캐시 엔트리의 data 배열의 offset 위치부터 데이터를 int 크기만큼(4바이트) 읽어옴
        num_bytes += 4;                                     // 지금까지 접근한 바이트 수를 뜻하는 num_bytes에 +4
        break;                                              // case 'w' 실행 후 switch문 밖으로 탈출
    }

    num_access_cycles += CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE;      // cache miss case. 캐시와 메모리에 둘 다 접근했으므로 access cycles 수에 cache access cycle + memory access cycle인 +101
    return accessed_data;                                               // 메모리 접근해 캐시로 복사한 데이터 accessed_data 반환
}
