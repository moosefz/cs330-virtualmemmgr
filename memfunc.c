//Names: Mustafa Fawaz (103184737) / Ashraf Taifour (104262768)
//Date: Mar 10 2020
//COMP-3300 - Project: Virtual Memory Manager
//PROJECT FUNCTION FILE
/** We created this header file so keeping track of each function called is easier and less work than scrolling through a large C file. */

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


  /* mmap() is a system call that stores the contents of the BACKING_STORE in contiguous memory
  returning the location in memory to the variable backStoreMap. This variable
  is used to access the contents of the backStore as needed.
  */
  //NOTE: mmap takes the following parameters: first parameter is the size we want to start mapping (virtual memory)
  //second parameter is the size we want to map
  //third and fourth parameter is any options about paging:PROT_READ will alow for reading, MAP_SHARED here means it will be visible for other processes.
  //fifth parameter is the .bin file and the sixth parameter is the offset (we're starting at 0)
  //return value is a pointer to the mapped area if succesful.
  backStoreMap = mmap(0, PM_SIZE, PROT_READ, MAP_SHARED, backStoreFD, 0);
  if(backStoreMap == MAP_FAILED) {
    close(backStoreFD);
    printf("Failed to map backing store. Exiting.\n");
    exit(1);
  }
}

/* Checks the TLB for existing page numbers and returns corresponding
  frame number */
//NOTE: TLB has 16 rows and 2 columns, the first column is the page number while the second column is the frame number.
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

  int temp = pageTable[pageNumber]; //temp var to store pageTable return.
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
    //NOTE: we % TLB_SIZE here in so we never overflow (SEG FAULT) our tlbBUffer's size. This means that we will eventually overwrite values in TLB if we already used all of its slots.
    tlbPointer = (++tlbPointer) % TLB_SIZE;

    //Enqueue entry into TLB from the back of the queue
    tlBuffer[tlbPointer][0] = pageNumber;
    tlBuffer[tlbPointer][1] = frameNum;
  }
}
