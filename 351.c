#include<stdio.h> 
#include <stdlib.h> 

#define FRAME_SIZE 256

int TLB[16][2];
int TLBsize = 0;
        
int pTable[FRAME_SIZE];

int memFull = 0;
int mem[FRAME_SIZE][FRAME_SIZE];

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
  
int main(int argc,char* argv[]) 
{ 
    if(argc==1) 
        printf("No file name is passed.\n"); 
    if(argc>=2) 
    { 
        char *fileName = argv[1];
        FILE *fp = fopen(fileName, "r"); 
        
        if (fp == NULL) 
        { 
            printf("Couldn't open file."); 
            return -1;
        } 

        int address;
        int pageFaultStat = 0, TLBstat = 0;
        int tAccess = 0;

        for(int i=0; i<FRAME_SIZE; i++){
            pTable[i] = -1;
        }

        while(fscanf(fp, "%d", &address ) != EOF){ 
            tAccess++;

            int pageNumMask = 0; 
            int offsetMask = 0;
            for(int i=0; i<8; i++){
                offsetMask |= 1 << i;
                pageNumMask |= 1 << (i+8);
            }

            int pageNum = pageNumMask & address;
            pageNum = pageNum >> 8;
            int offset = offsetMask & address;

            int TLBhit = 0;

            for(int i=0; i<TLBsize; i++){
                if(TLB[i][0] == pageNum){
                    TLBhit = 1;
                    int phyAddress = TLB[i][1]*FRAME_SIZE + offset;
                    printf("Virtual Address : %d and Physical Address : %d\n", address, phyAddress);
                    TLBstat++;
                    break;
                }
            }

            if (TLBhit == 0){
                if(pTable[pageNum] == -1){
                    pageFault(pageNum);
                    pageFaultStat++;
                }
                int phyAddress = pTable[pageNum]*FRAME_SIZE + offset;
                printf("Virtual Address : %d and Physical Address : %d\n", address, phyAddress);

                if(TLBsize != 16){
                    TLB[TLBsize][0] = pageNum;
                    TLB[TLBsize][1] = pTable[pageNum];
                    TLBsize++;
                }
                else{
                    for(int i=0; i<15; i++){
                        TLB[i][0] = TLB[i+1][0];
                        TLB[i][1] = TLB[i+1][1];
                    }
                    TLB[15][0] = pageNum;
                    TLB[15][1] = pTable[pageNum];
                }
            }
        }
        fclose(fp); 
    } 
    return 0; 
} 