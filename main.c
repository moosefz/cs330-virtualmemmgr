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
  //the following function calls will initialize the arrays that act as the pagetable and tlb to -1 (implying empty)
  //while initializing the backstore the given address values in the .bin file.

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
  //Page table additions and hits/faults will occur within this loop
  while( !feof(addressFile) ) { //loop will end only when EOF is reached in the address file.


    fgets(address, 7, addressFile); //will take an address that is 7(including newline) characters long, this will go line by line through the address file and store it in the address variable.
    sscanf(address, "%d", &currentAddress); //will convert the address variable into an int and store it in currentAddress

    //Calculate offset and page number of each address
    offset = currentAddress & 0xff; //&ing the value of the address that we have with 11111111 this will preserve the value of the first 8 bits.
    pageNumber = currentAddress >> 8; //then we bit shift to the left by 8 bits, and this gives us the the bits that were at position 8-15 which represents the page number.

    //Check TLB for pageNumber
    //initially TLB is checked as it will make the process faster if this entry has already been done before.
    frameNum = checkTLB(pageNumber);

    /* With demand paging, the first series of pages will fault because
    the TLB will be empty. This will change overtime as the TLB fills. */

    //TLB Hit (-1 == TLB Miss)
    if(frameNum != -1) { //If it exists in TLB

      //Determine physical address
      physicalAddress = frameNum + offset; //adding offset and framenumber will give us the physical address
      byteValue = physicalMem[physicalAddress];//we pass the physicalAddress into our physical memory array to obtain the value in that address and store it in 'byteValue'.

    //TLB Miss, check Page Table
    } else {

      //Checking Page table for Frame
      frameNum = checkPageTable(pageNumber);

      //No page fault, frame exists
      if(frameNum != -1) {

        //Calculate physical address and add it to the TLB
        physicalAddress = frameNum + offset;
        addToTLB(frameNum, pageNumber); //frameNum is added to the first column, pageNumber to the second column
        byteValue = physicalMem[physicalAddress];

        //PAGE FAULT
      } else {

        //Acquire front of the pageTable
        int pageFront = pageNumber * PAGE_SIZE;

        //Find a free frame
	//NOTE: phyysMemPointer is a variable that is initialized in memfunc.c to equal 0, it is used as an index for physical memory.
        if(physMemPointer != -1) {

          //Page retrieved from backing_store.bin and into the free frame
          //memcpy(dest, src, size) - this will copy data from the .bin file into the physicalMem array. the size of the entry will be PAGE_SIZE = 256
          memcpy(physicalMem + physMemPointer, backStoreMap + pageFront, PAGE_SIZE);

          //Determine physical address and value output based on backing_store addition
          frameNum = physMemPointer; //frameNum will now point to its dedicated memory.
          physicalAddress = frameNum + offset; //the address will be the offset added to the frameNum
          byteValue = physicalMem[physicalAddress]; //to access the value we just pass the physicalAddress to our physical storage array.

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

  finish = clock();
  // the start and finish numbers are the number of clock ticks elapsed since the program was launched. dividing by CLOCKS_PER_SEC will give us the number of seconds used by the CPU
  loop_time = finish - start;
  printf("loop time = %ld clock ticks\n", loop_time);
  total_time = ((double) (finish - start)) / CLOCKS_PER_SEC;
  //number of seconds will be 0 on our machine as the program is executed too quickly

  /* WRITE TO OUTPUT FILE HERE */
  fprintf(outputFile, "Number of Translated Addresses: %d\n", addressCount);
  fprintf(outputFile, "Page Faults = %d\n", pageFaultCounter);
  fprintf(outputFile, "Page Fault Rate = %.3f\n", (double)pageFaultCounter/addressCount);
  fprintf(outputFile, "TLB Hits = %d\n", TLBCounter);
  fprintf(outputFile, "TLB Hit Rate = %.3f\n", (double)TLBCounter/addressCount);
  fprintf(outputFile, "Average Page-Replacement Time = %.9f", (total_time)/1000.0);
}
