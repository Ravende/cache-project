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

int retrieve_data(void *addr, char data_type) {                 // input �������� �ּҿ� ������ Ÿ���� �Ķ���ͷ� �޾� cache hit/miss ���ο� ���� �˸��� �����͸� ��ȯ�ϴ� retrieve_data �Լ�
    int value_returned = -1; /* accessed data */                // ��ȯ�ϴ� ������ ���� �����ϴ� value_returned ����, �ʱⰪ -1

    /* Invoke check_cache_data_hit() */
    value_returned = check_cache_data_hit(addr, data_type);     // check_cache_data_hit �Լ��� �ҷ��� �� ��ȯ���� value_returned ������ ����. cache hit �� ĳ�� ���� ������, miss �� -1

    /* In case of the cache miss event, access the main memory by invoking access_memory() */
    if (value_returned == -1) {                                 // value_returned�� -1�� �� (= cache miss�� ��)
        value_returned = access_memory(addr, data_type);        // access_memory �Լ��� �ҷ� ���� �޸𸮿� �����ؼ� ������ �����͸� value_returned ������ ����
    }

    global_timestamp++;         // ������ ������ ������ ���� timestamp�� 1 ������Ű��
    return value_returned;      // value_returned ��ȯ
}

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
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
    char buffer[256];                                                   // ���Ͽ��� �о�� ���ڿ��� �ӽ÷� �����ϱ� ���� ������ ũ���� char �迭
    fprintf(ofp, "[Accessed Data]\n");                                  // access_output.txt ���Ͽ� "[Accessed Data]\n" ���
    while (fgets(buffer, sizeof(buffer), ifp) != NULL) {                // access_input.txt ���Ͽ��� ���پ� �о�ͼ� buffer�� �����ϴ� ���� ����. ���� ���� ������ �� (NULL) ���� ����
        (void)sscanf(buffer, "%lu %c", &access_addr, &access_type);     // ���ڿ� buffer���� ������ ����. long unsigned int Ÿ�� �����ʹ� access_addr��, char Ÿ�� �����ʹ� access_type�� ���� ����
        
        accessed_data = retrieve_data(&access_addr, access_type);                       // �� �ڵ带 ���� input ���Ͽ��� ������ �ּҿ� ������ Ÿ���� ���� retrieve_data�� �Ķ���ͷ� �־� �Լ� ȣ��, ��ȯ�� �����͸� accessed_data�� ����
        fprintf(ofp, "%lu\t\t%c\t\t%#x\n", access_addr, access_type, accessed_data);    // access_output.txt ���Ͽ� access_addr, access_type, accessed_data (�ּ�, ������ Ÿ��, ���� ������) ���ʷ� ��� (while�� �ȿ��� �ݺ�)
    }

    fprintf(ofp, "------------------------------------------------\n");     // access_output.txt ���Ͽ� "------------------------------------------------\n" ���
    switch (DEFAULT_CACHE_ASSOC) {                                          // ������ ĳ���� associativity�� ���� �Ǵ��ؼ� ����ϴ� switch��
    case 1:                                                                 // associativity = 1, �� direct mapped cache�� ���
        fprintf(ofp, "[Direct mapped cache performance]\n");                // access_output.txt ���Ͽ� "[Direct mapped cache performance]\n" ���
        break;                                                              // case 1 ���� �� switch�� ������ Ż��
    case 2:                                                                 // associativity = 2, �� 2-way set associative cache�� ���
        fprintf(ofp, "[2-way set associative cache performance]\n");        // access_output.txt ���Ͽ� "[2-way set associative cache performance]\n" ���
        break;                                                              // case 2 ���� �� switch�� ������ Ż��
    case 4:                                                                 // associativity = 4, �� fully associative cache�� ���
        fprintf(ofp, "[Fully associative cache performance]\n");            // access_output.txt ���Ͽ� "[Fully associative cache performance]\n" ���
        break;                                                              // case 4 ���� �� switch�� ������ Ż��
    }
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", (float)num_cache_hits/global_timestamp, num_cache_hits, global_timestamp);   // access_output.txt ���Ͽ� cache hit ���� ���� timestamp�� ���� ����� hit ratio ���
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", (float)num_bytes / num_access_cycles, num_bytes, num_access_cycles);         // access_output.txt ���Ͽ� ���ݱ��� ������ byte ���� ���ݱ����� access cycle ���� ���� ����� bandwidth ���
    

    fclose(ifp);    // access_input.txt ���� �ݱ�
    fclose(ofp);    // access_output.txt ���� �ݱ�
    
    print_cache_entries();      // print_cache_entries() �Լ��� ĳ�� ��Ʈ�� ���� ���
    return 0;                   // �Լ� ������ 0 ����
}
