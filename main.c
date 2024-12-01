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

int retrieve_data(void *addr, char data_type) {                 // input 데이터의 주소와 데이터 타입을 파라미터로 받아 cache hit/miss 여부에 따른 알맞은 데이터를 반환하는 retrieve_data 함수
    int value_returned = -1; /* accessed data */                // 반환하는 데이터 값을 저장하는 value_returned 선언, 초기값 -1

    /* Invoke check_cache_data_hit() */
    value_returned = check_cache_data_hit(addr, data_type);     // check_cache_data_hit 함수를 불러서 그 반환값을 value_returned 변수에 저장. cache hit 시 캐시 안의 데이터, miss 시 -1

    /* In case of the cache miss event, access the main memory by invoking access_memory() */
    if (value_returned == -1) {                                 // value_returned가 -1일 때 (= cache miss일 때)
        value_returned = access_memory(addr, data_type);        // access_memory 함수를 불러 메인 메모리에 접근해서 가져온 데이터를 value_returned 변수에 저장
    }

    global_timestamp++;         // 데이터 접근할 때마다 전역 timestamp를 1 증가시키기
    return value_returned;      // value_returned 반환
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
    char buffer[256];                                                   // 파일에서 읽어온 문자열을 임시로 저장하기 위한 적당한 크기의 char 배열
    fprintf(ofp, "[Accessed Data]\n");                                  // access_output.txt 파일에 "[Accessed Data]\n" 출력
    while (fgets(buffer, sizeof(buffer), ifp) != NULL) {                // access_input.txt 파일에서 한줄씩 읽어와서 buffer에 저장하는 무한 루프. 파일 끝에 도달할 시 (NULL) 루프 종료
        (void)sscanf(buffer, "%lu %c", &access_addr, &access_type);     // 문자열 buffer에서 데이터 추출. long unsigned int 타입 데이터는 access_addr에, char 타입 데이터는 access_type에 각각 저장
        
        accessed_data = retrieve_data(&access_addr, access_type);                       // 위 코드를 통해 input 파일에서 추출한 주소와 데이터 타입을 각각 retrieve_data에 파라미터로 넣어 함수 호출, 반환한 데이터를 accessed_data에 저장
        fprintf(ofp, "%lu\t\t%c\t\t%#x\n", access_addr, access_type, accessed_data);    // access_output.txt 파일에 access_addr, access_type, accessed_data (주소, 데이터 타입, 실제 데이터) 차례로 출력 (while문 안에서 반복)
    }

    fprintf(ofp, "------------------------------------------------\n");     // access_output.txt 파일에 "------------------------------------------------\n" 출력
    switch (DEFAULT_CACHE_ASSOC) {                                          // 설정된 캐시의 associativity에 따라 판단해서 출력하는 switch문
    case 1:                                                                 // associativity = 1, 즉 direct mapped cache일 경우
        fprintf(ofp, "[Direct mapped cache performance]\n");                // access_output.txt 파일에 "[Direct mapped cache performance]\n" 출력
        break;                                                              // case 1 실행 후 switch문 밖으로 탈출
    case 2:                                                                 // associativity = 2, 즉 2-way set associative cache일 경우
        fprintf(ofp, "[2-way set associative cache performance]\n");        // access_output.txt 파일에 "[2-way set associative cache performance]\n" 출력
        break;                                                              // case 2 실행 후 switch문 밖으로 탈출
    case 4:                                                                 // associativity = 4, 즉 fully associative cache일 경우
        fprintf(ofp, "[Fully associative cache performance]\n");            // access_output.txt 파일에 "[Fully associative cache performance]\n" 출력
        break;                                                              // case 4 실행 후 switch문 밖으로 탈출
    }
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", (float)num_cache_hits/global_timestamp, num_cache_hits, global_timestamp);   // access_output.txt 파일에 cache hit 수를 전역 timestamp로 나눠 계산한 hit ratio 출력
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", (float)num_bytes / num_access_cycles, num_bytes, num_access_cycles);         // access_output.txt 파일에 지금까지 접근한 byte 수를 지금까지의 access cycle 수로 나눠 계산한 bandwidth 출력
    

    fclose(ifp);    // access_input.txt 파일 닫기
    fclose(ofp);    // access_output.txt 파일 닫기
    
    print_cache_entries();      // print_cache_entries() 함수로 캐시 엔트리 내역 출력
    return 0;                   // 함수 끝에서 0 리턴
}
