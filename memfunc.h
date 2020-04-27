//Names: Mustafa Fawaz (103184737) / Ashraf Taifour (104262768)
//Date: Mar 10 2020
//COMP-3300 - Project: Virtual Memory Manager
//PROJECT HEADER FILE

//====================================
//ALL MACRO DEFINITIONS
//====================================
#define PAGE_SIZE 256
#define PM_SIZE (256*256) //256 frames * 256 bytes
#define TLB_SIZE 16

//=====================================
//ALL FUNCTION PROTOTYPES (SEE memfunc.c)
//=====================================
void createPageTable();
void createTLB();
void createBackStore();
int checkTLB(int); //Used to check the TLB for corresponding page number
int checkPageTable(int);
void addToTLB(int, int);
