//Names: Mustafa Fawaz (103184737) / Ashraf Taifour (104262768)
//Date: Mar 10 2020
//COMP-3300 - Project: Virtual Memory Manager
//PROJECT FUNCTION FILE

//==========================
//ALL DATA STRUCTURES
//==========================
char physicalMem[PM_SIZE];
int pageTable[PAGE_SIZE];
int tlBuffer[TLB_SIZE][2];

//==========================
//ALL GLOBAL VARIABLES
//==========================
int TLBCounter = 0, pageFaultCounter = 0, addressCount = 0;
char *backStoreMap;
int backStoreFD; //Used as the file descriptor for the backStore
int tlbPointer = -1;
int physMemPointer = 0; //used an index for physical memory

//==========================
//ALL FUNCTION BODIES
//==========================

/* Creates page table data structure */
void createPageTable() {
  //Page table is initialized as empty
  for(int i = 0; i < PAGE_SIZE; i++)
    pageTable[i] = -1;
}

/* Initializes the TLB, -1 used as placeholder for empties */
void createTLB() {
  for(int i = 0; i < TLB_SIZE; i++) {
    tlBuffer[i][0] = -1; //Page # Column
    tlBuffer[i][1] = -1; //Frame # Column
  }
}

/* Creates backing store and maps to memory */
void createBackStore() {
  //Backing Store file is checked in directory and opened
  if((backStoreFD = open("BACKING_STORE.bin", O_RDONLY)) == -1) {
    printf("BACKING STORE file not found. Please add to directory.\n");
    exit(1);
  }
  
  backStoreMap = mmap(0, PM_SIZE, PROT_READ, MAP_SHARED, backStoreFD, 0);
  if(backStoreMap == MAP_FAILED) {
    close(backStoreFD);
    printf("Failed to map backing store. Exiting.\n");
    exit(1);
  }
}

/* Checks the TLB for existing page numbers and returns corresponding
  frame number */
int checkTLB(int pageNumber) {
  for(int i = 0; i < TLB_SIZE; i++)
  {
    if(tlBuffer[i][0] == pageNumber) 
    {
      ++TLBCounter; //used for TLB hit counter
      return tlBuffer[i][1]; //return corresponding frame #
    }
  }
    return -1; //TLB Miss
}

/* Checks page table if TLB miss occurs. Either found or not found in pageTable.
 If not found, return -1 and consult BACKING_STORE*/
int checkPageTable(int pageNumber) {

  int temp = pageTable[pageNumber]; 
  //Page Fault - increment fault counter for rates
  if(temp == -1) {
    ++pageFaultCounter;
  }
  //Return the frame regardless of hit or miss
  return temp;
}

/* Adds/Updates Frame Number and Page Number to TLB using FIFO */
void addToTLB(int frameNum, int pageNumber) {

//if TLB is empty
  if(tlbPointer == -1) {
    tlbPointer = 0;

    //Update TLB
    tlBuffer[tlbPointer][0] = pageNumber;
    tlBuffer[tlbPointer][1] = frameNum;

  //if TLB is not empty, all elements must be shifted and new entry added to back
  } else {
    //Set TLB access point to end of queue (empty to start)
    tlbPointer = (++tlbPointer) % TLB_SIZE;

    //Enqueue entry into TLB from the back of the queue
    tlBuffer[tlbPointer][0] = pageNumber;
    tlBuffer[tlbPointer][1] = frameNum;
  }
}
