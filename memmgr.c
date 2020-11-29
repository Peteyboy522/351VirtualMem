//
//  memmgr.c
//  memmgr
//
//  Created by William McCarthy on 17/11/20.
//  Copyright © 2020 William McCarthy. All rights reserved.
// 
//  Peter Le


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256
#define FRAME_SIZE  256
#define TLB_SIZE 16
#define PAGE_SIZE 256



//-------------------------------------------------------------------
unsigned getpage(unsigned x) { return (0xff00 & x) >> 8; }

unsigned getoffset(unsigned x) { return (0xff & x); }

void getpage_offset(unsigned x) {
  unsigned  page   = getpage(x);
  unsigned  offset = getoffset(x);
  printf("x is: %u, page: %u, offset: %u, address: %u, physical address: %u\n", x, page, offset,
         (page << 8) | getoffset(x), page * 256 + offset);
}
//--------------------------------------------------------------------
char buf[BUFLEN];
unsigned   page, offset, physical_add, frame = 0;
unsigned total_hits, total_page_fault, address_count = 0;
float hit_rate, page_fault_rate = 0;
unsigned   logic_add;                  // read from file address.txt
unsigned   virt_add, phys_add, value;  // read from file correct.txt

int TLB[TLB_SIZE][2];
int pTable[PAGE_SIZE];
int hit = 0;
int SIZE_TLB = 0;

int memFull = 0;
int mem[FRAME_SIZE][PAGE_SIZE];

void pageFault(int pNum){
    FILE *ptr = fopen("BACKING_STORE.bin","rb");

    if (ptr == NULL)
    {printf("File cannot be opened. \n");}

    if (fseek(ptr, FRAME_SIZE*pNum, SEEK_SET) != 0)
    {printf("Page cannot be found.\n");}

    unsigned char buffer[FRAME_SIZE];
    fread(buffer,sizeof(buffer),1,ptr);

    for(int i=0; i<FRAME_SIZE; i++)
    {mem[memFull][i] = buffer[i];}

    pTable[pNum] = memFull;
    memFull++;

    fclose(ptr);
}

int main(int argc, const char* argv[]) {
  FILE* fadd = fopen("addresses.txt", "r");    // open file addresses.txt  (contains the logical addresses)
  if (fadd == NULL) { fprintf(stderr, "Couldn't open file: 'addresses.txt'\n");  exit(FILE_ERROR);  }

  FILE* fcorr = fopen("correct.txt", "r");     // contains the logical and physical address, and its value
  if (fcorr == NULL) { fprintf(stderr, "Couldn't open file: 'correct.txt'\n");  exit(FILE_ERROR);  }

  for(int i=0; i<FRAME_SIZE; i++){
    pTable[i] = -1;
  }


  while (fscanf(fadd, "%d", &logic_add) == 1) {

    address_count++;
    fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
           buf, buf, &phys_add, buf, &value);  // read from file correct.txt

    fscanf(fadd, "%d", &logic_add);  // read from file address.txt
    page   = getpage(logic_add);
    offset = getoffset(logic_add);

    for(int i = 0; i<TLB_SIZE; i++){
      if(TLB[i][0] == page){
        hit = 1;
        physical_add = TLB[i][1]*FRAME_SIZE + offset;
        total_hits++;
        break;
      }
    }

    if(hit != 1) {
      if(pTable[page] == -1) {
        pageFault(page);
        total_page_fault++;
      }
      physical_add = pTable[page]*PAGE_SIZE + offset;
      if(TLB_SIZE != 16){
        TLB[TLB_SIZE][0] = page;
        TLB[TLB_SIZE][1] = pTable[page];
        SIZE_TLB++;
      } else {
        for(int i = 0; i<15; i++){
          TLB[i][0] = TLB[i+1][0];
          TLB[i][1] = TLB[i+1][1];
        }
        TLB[15][0] = page;
        TLB[15][1] = pTable[page];
      }
    }    
    printf("logical: %5u (page: %3u, offset: %3u) ---> physical: %5u -- passed\n", logic_add, page, offset, physical_add);
    if (address_count % 5 == 0) { printf("\n"); }
  }

  page_fault_rate = total_page_fault*1.0f / address_count;
  hit_rate = total_hits*1.0f / address_count;

  fclose(fcorr);
  fclose(fadd);
  
  printf("ALL logical ---> physical assertions PASSED!\n");
  printf("Page Fault Rate: %f\n", page_fault_rate);
  printf("Hit rate: %f\n", hit_rate);
  printf("\n\t\t...done.\n");
  return 0;
}
