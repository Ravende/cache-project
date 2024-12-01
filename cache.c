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

int check_cache_data_hit(void *addr, char type) {    // cache�� hit ���θ� ã�� �Լ�. cache hit �� cache���� �ٷ� ���� �����͸�, miss �� -1�� ��ȯ�Ѵ�.

    /* Fill out here */

    int byte_address = *((int*)addr);                                  // void Ÿ���� addr�� int Ÿ������ ��ȯ�� byte_address�� ���� (���� ����)
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;  // byte_address�� block size(8)�� ���� block_address�� ��ȯ��
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;   // byte_address�� blcok size(8)�� ������ ������ �������� ���� block_offset ����

    int cache_index = block_address % CACHE_SET_SIZE;  // block_address�� set size�� ���� �������� ���� (modulo ����) cache index ����
    int tag = block_address / CACHE_SET_SIZE;          // block_address�� set size�� ���� ���� ���� tag ����

    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {             // i�� ĳ���� associativity��ŭ ���鼭 �ش� set�� entry �ϳ��� ���� Ȯ��
        cache_entry_t* pEntry = &cache_array[cache_index][i];   // cache_array �迭�� ����� cache index�� set index��, i�� entry index�� �־ cache_entry_t ����ü pEntry ����
        if (pEntry->valid == 1 && pEntry->tag == tag) {         // �ش� entry�� valid bit�� 1����, tag�� ������ ����� tag�� ��ġ�ϴ��� �Բ� Ȯ�� => cache hit���� Ȯ��. �� �� ������ �� hit ����

            int cache_data = -1;                                    // ����ü(ĳ�� ��Ʈ��)���� �������� �����͸� ���� cache_data (�ʱⰪ -1)
            int offset = block_offset;                              // ĳ���� �����͸� ��� ����Ʈ���� �������� �����ϱ� ���� offset: ������ ����� block_offset���� ����
            switch (type) {                                         // �Լ� �Ķ���ͷ� ���޹��� type(input�� ������ Ÿ��)�� ���� �Ǵ��ϴ� switch��
            case 'b':                                               // type�� 'b', �� ����Ʈ(1byte)�� ���
                cache_data = pEntry->data[offset];                  // ĳ�� ��Ʈ���� data �迭�� offset ��ġ���� �����͸� 1����Ʈ �о��
                num_bytes++;                                        // ���ݱ��� ������ ����Ʈ ���� ���ϴ� num_bytes�� +1
                break;                                              // case 'b' ���� �� switch�� ������ Ż��
            case 'h':                                               // type�� 'h', �� ��������(2byte)�� ���
                cache_data = *((short*)&pEntry->data[offset]);      // short* Ÿ������ �ּ� ��ȯ. => ĳ�� ��Ʈ���� data �迭�� offset ��ġ���� �����͸� short ũ�⸸ŭ(2����Ʈ) �о��
                num_bytes += 2;                                     // ���ݱ��� ������ ����Ʈ ���� ���ϴ� num_bytes�� +2
                break;                                              // case 'h' ���� �� switch�� ������ Ż��
            case 'w':                                               // type�� 'w', �� ����(4byte)�� ���
                cache_data = *((int*)&pEntry->data[offset]);        // int* Ÿ������ �ּ� ��ȯ. => ĳ�� ��Ʈ���� data �迭�� offset ��ġ���� �����͸� int ũ�⸸ŭ(4����Ʈ) �о��
                num_bytes += 4;                                     // ���ݱ��� ������ ����Ʈ ���� ���ϴ� num_bytes�� +4
                break;                                              // case 'w' ���� �� switch�� ������ Ż��
            }

            pEntry->timestamp = global_timestamp;       // �ش� ĳ�� ��Ʈ���� timestamp�� ���ݱ����� ���� timestamp ���� ������ ����
            num_cache_hits++;                           // cache hit �� +1
            num_access_cycles += CACHE_ACCESS_CYCLE;    // cache hit�̹Ƿ� access cycles ���� cache access cycle�� +1
            return cache_data;                          // ĳ�÷κ��� �о�� ������ ��ȯ
        }
    }
    num_cache_misses++;      // �ش� cache index(set index)�� ��� entry���� cache hit�� ���� ������ ��� cache miss. cache miss �� +1

    /* Return the data */
    return -1;               // cache miss�� ��� -1 ����
}

int find_entry_index_in_set(int cache_index) {    // cache index�� �־����� �ش� set�� entry_index�� ã�� �Լ�
    int entry_index = 0;                          // entry_index ����. �ʱⰪ 0���� ����

    /* Check if there exists any empty cache space by checking 'valid' */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {             // i�� ĳ���� associativity��ŭ ���鼭 �ش� set�� entry �ϳ��� ���� Ȯ��
        cache_entry_t* pEntry = &cache_array[cache_index][i];   // cache_array �迭�� �Ķ���ͷ� �޾ƿ� cache index�� set index��, i�� entry index�� �־ cache_entry_t ����ü pEntry�� ����
        if (pEntry->valid == 0) {                               // ���� pEntry�� valid bit�� 0�̸� (= �� ��Ʈ���� ������)
            return i;                                           // �ش� entry�� �ε��� i�� �ٷ� ����
        }
    }

    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    int least_recent_used_timestamp = cache_array[cache_index][0].timestamp;    // �� entry�� ���� ���. ����� �� ���� ������ ��Ʈ���� timestamp �����ϴ� ���� - �ʱⰪ���� entry 0�� timestamp ����
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {                             // i�� ĳ���� associativity��ŭ ���鼭 �ش� set�� entry �ϳ��� ���� Ȯ��
        cache_entry_t* pEntry = &cache_array[cache_index][i];                   // cache_array �迭�� �Ķ���ͷ� �޾ƿ� cache index�� set index��, i�� entry index�� �־ cache_entry_t ����ü pEntry�� ����
        if (pEntry->timestamp < least_recent_used_timestamp) {                  // �ش� entry�� timestamp(��� ������ ���Ǿ����� �Ǻ�)�� least_recent_used_timestamp���� ������ (= �� �����Ǿ�����)
            least_recent_used_timestamp = pEntry->timestamp;                    // least_recent_used_timestamp ������ �ش� entry�� timestamp�� �����ϰ�
            entry_index = i;                                                    // entry_index���� �ش� entry�� �ε��� i�� �����Ѵ�.
        }
    }

    return entry_index;     // for���� ������ LRU timestamp�� entry_index�� ����. entry_index ��ȯ�Ѵ�
}

int access_memory(void* addr, char type) {      // (cache miss�� ���,) input�� �ּҿ� ������ Ÿ���� �Ķ���ͷ� �޾� �޸𸮿� �����ϰ� ���� �����͸� ��ȯ���ִ� �Լ�

    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */
    int byte_address = *((int*)addr);                                   // void Ÿ���� addr�� int Ÿ������ ��ȯ�� byte_address�� ���� (���� ����)
    int block_address = byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE;   // byte_address�� block size(8)�� ���� block_address�� ��ȯ��
    int block_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;    // byte_address�� blcok size(8)�� ������ ������ �������� ���� block_offset ����

    int cache_index = block_address % CACHE_SET_SIZE;                   // block_address�� set size�� ���� �������� ���� (modulo ����) cache index ����

    /* You need to invoke find_entry_index_in_set() for copying to the cache */
    int entry_index = find_entry_index_in_set(cache_index);             // ����� cache_index�� �Ķ���ͷ� ���� find_entry_index_in_set �Լ��� �ҷ� entry_index�� ã��

    cache_entry_t* pEntry = &cache_array[cache_index][entry_index];     // cache_array �迭�� ����� cache index�� set �ε�����, �Ķ���ͷ� �޾ƿ� entry_index�� entry �ε����� �־ cache_entry_t ����ü pEntry�� ���� (�޸��� ������ copy�� ĳ�� ��Ʈ��)
    pEntry->valid = 1;                                                  // �ش� entry�� valid bit�� 1�� ����
    pEntry->tag = block_address / CACHE_SET_SIZE;                       // �ش� entry�� tag�� block_address�� set size�� ���� ������ ����
    pEntry->timestamp = global_timestamp;                               // �ش� entry�� timestamp�� ���ݱ����� ���� timestamp ���� ������ ����

    char* ptr = (char*)memory_array;                                    // memory_array�� char(1����Ʈ) ������ �����ϱ� ���� char* Ÿ������ ĳ�����Ͽ� char* ptr ������ ������ ����

    int start_of_memory_address = block_address * DEFAULT_CACHE_BLOCK_SIZE_BYTE;        // �о�� �޸��� �ּ�(����Ʈ ����)�� �����ϱ� ���� block address�� block size(8)�� ���ؼ� ���
    for (int i = 0; i < DEFAULT_CACHE_BLOCK_SIZE_BYTE; i++) {                           // for���� block size(8) ũ�⸸ŭ ���鼭
        pEntry->data[i] = ptr[start_of_memory_address + i];                             // pEntry�� data�� i�ּ�(0~7)�� memory_array�� start_of_memory_address ��ġ���� i�� �����ָ� �� ����Ʈ��, ��� 2���� ����
    }

    /* Return the accessed data with a suitable type */
    int accessed_data = -1;                                 // �޸𸮿��� ������ (�����δ� �ش� �޸��� �����͸� ������ ĳ�� ��Ʈ������ ��������) �����͸� ���� accessed_data (�ʱⰪ -1)
    int offset = block_offset;                              // ĳ���� �����͸� ��� ����Ʈ���� �������� �����ϱ� ���� offset: ������ ����� block_offset���� ����
    switch (type) {                                         // �Լ� �Ķ���ͷ� ���޹��� type(input�� ������ Ÿ��)�� ���� �Ǵ��ϴ� switch��
    case 'b':                                               // type�� 'b', �� ����Ʈ(1byte)�� ���
        accessed_data = pEntry->data[offset];               // ĳ�� ��Ʈ���� data �迭�� offset ��ġ���� �����͸� 1����Ʈ �о��
        num_bytes++;                                        // ���ݱ��� ������ ����Ʈ ���� ���ϴ� num_bytes�� +1
        break;                                              // case 'b' ���� �� switch�� ������ Ż��
    case 'h':                                               // type�� 'h', �� ��������(2byte)�� ���
        accessed_data = *((short*)&pEntry->data[offset]);   // short* Ÿ������ �ּ� ��ȯ. => ĳ�� ��Ʈ���� data �迭�� offset ��ġ���� �����͸� short ũ�⸸ŭ(2����Ʈ) �о��
        num_bytes += 2;                                     // ���ݱ��� ������ ����Ʈ ���� ���ϴ� num_bytes�� +2
        break;                                              // case 'h' ���� �� switch�� ������ Ż��
    case 'w':                                               // type�� 'w', �� ����(4byte)�� ���
        accessed_data = *((int*)&pEntry->data[offset]);     // int* Ÿ������ �ּ� ��ȯ. => ĳ�� ��Ʈ���� data �迭�� offset ��ġ���� �����͸� int ũ�⸸ŭ(4����Ʈ) �о��
        num_bytes += 4;                                     // ���ݱ��� ������ ����Ʈ ���� ���ϴ� num_bytes�� +4
        break;                                              // case 'w' ���� �� switch�� ������ Ż��
    }

    num_access_cycles += CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE;      // cache miss case. ĳ�ÿ� �޸𸮿� �� �� ���������Ƿ� access cycles ���� cache access cycle + memory access cycle�� +101
    return accessed_data;                                               // �޸� ������ ĳ�÷� ������ ������ accessed_data ��ȯ
}
