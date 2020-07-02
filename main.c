//Names: Mustafa Fawaz (103184737) / Ashraf Taifour (104262768)
//Date: Mar 10 2020
//COMP-3300 - Project: Virtual Memory Manager

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "memfunc.h" //header file
#include "memfunc.c" //function file

int main(int argc, char *argv[]) {
  FILE *addressFile; //Used as the input file for the addresses
  FILE *outputFile;
  char address[7];
  int currentAddress, physicalAddress;
  int pageNumber, offset, frameNum, byteValue, tempVal;
  clock_t start, finish, loop_time, total_time;

  /* INITIALIZE ALL DATA STRUCTURES - Functions available on memfunc.c */
  createPageTable();
  createTLB();
  createBackStore();

	/* OPEN ADDRESSES.TXT AND OUTPUT FILE*/
  //Input file to be opened with error checking
  if((addressFile = fopen(argv[1], "r")) == NULL) {
    printf("Error Opening file: %s\n", argv[1]);
    exit(1);
  }

  //Create output file that will be written to.
  if((outputFile = fopen("output.txt", "w")) == NULL) {
    printf("Error creating output file. Exiting.\n");
    exit(1);
  }
	
  start = clock(); //Begin clock counter
  
  //Parse address file and output respective page numbers and offsets
  while( !feof(addressFile) ) { 

    // retrieve address from file
    fgets(address, 7, addressFile); 
    sscanf(address, "%d", &currentAddress);

    //Calculate offset and page number of each address
    offset = currentAddress & 0xff; 
    pageNumber = currentAddress >> 8; 
	  
    //Check TLB for pageNumber
    frameNum = checkTLB(pageNumber);

    /* With demand paging, the first series of pages will fault because
    the TLB will be empty. This will change overtime as the TLB fills. */

    //TLB Hit (-1 == TLB Miss)
    if(frameNum != -1) { //If it exists in TLB

      //Determine physical address
      physicalAddress = frameNum + offset; 
      byteValue = physicalMem[physicalAddress];

    //TLB Miss, check Page Table
    } else {

      
      frameNum = checkPageTable(pageNumber);

      //No page fault, frame exists
      if(frameNum != -1) {

        //Calculate physical address and add it to the TLB
        physicalAddress = frameNum + offset;
        addToTLB(frameNum, pageNumber); 
        byteValue = physicalMem[physicalAddress];

        //PAGE FAULT
      } else {

        //Acquire front of the pageTable
        int pageFront = pageNumber * PAGE_SIZE;

        //Find a free frame
	//NOTE: phyysMemPointer is a variable that is initialized in memfunc.c to equal 0, it is used as an index for physical memory.
        if(physMemPointer != -1) {

          //Page retrieved from backing_store.bin and into the free frame
          memcpy(physicalMem + physMemPointer, backStoreMap + pageFront, PAGE_SIZE);

          //Determine physical address and value output based on backing_store addition
          frameNum = physMemPointer; 
          physicalAddress = frameNum + offset; 
          byteValue = physicalMem[physicalAddress]; 

          //Insert frame number into page table and update TLB, now this value will exist.
          pageTable[pageNumber] = physMemPointer;
          addToTLB(frameNum, pageNumber);

          /* Track memory index pointer for each addition to determine if room
          exists within the physical memory store. This is done by adding 256 bytes
          to the pointer from it's existing position to the next entry. If
          physical memory is full, assign the pointer to -1 indiciating no
          free frames */
          if(physMemPointer < PM_SIZE - 256) {
            physMemPointer += 256;
          } else {
            physMemPointer = -1;
          }
        }
      }
    }


    /* WRITING EACH VIRTUAL AND PHYSICAL ADDRESS */
    fprintf(outputFile, "Virtual Address: %d | ", currentAddress);
    fprintf(outputFile, "Physical Address: %d | ", physicalAddress);
    fprintf(outputFile, "Value: %d\n", byteValue);
    addressCount++; //Count number of addresses manipulated
  }
  
  // end clock counter & calclulate total time
  finish = clock();
  loop_time = finish - start;
  printf("loop time = %ld clock ticks\n", loop_time);
  total_time = ((double) (finish - start)) / CLOCKS_PER_SEC;
  

  /* WRITE TO OUTPUT FILE HERE */
  fprintf(outputFile, "Number of Translated Addresses: %d\n", addressCount);
  fprintf(outputFile, "Page Faults = %d\n", pageFaultCounter);
  fprintf(outputFile, "Page Fault Rate = %.3f\n", (double)pageFaultCounter/addressCount);
  fprintf(outputFile, "TLB Hits = %d\n", TLBCounter);
  fprintf(outputFile, "TLB Hit Rate = %.3f\n", (double)TLBCounter/addressCount);
  fprintf(outputFile, "Average Page-Replacement Time = %.9f", (total_time)/1000.0);
}
