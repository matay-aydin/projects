#ifndef _ALLOC
#define _ALLOC

#define BF (0)
#define WF (1)
#define FF (2)
#define NF (3)
#define MAGIC (123456789)

typedef struct __FreeNode {
    struct __FreeNode *next;
    int size;
} FreeNode;

typedef struct __Header {
    int magic;
    int size;
} Header;

extern int InitCalled;
extern unsigned long int HEAP_SIZE;
extern FreeNode *FreeHead;
extern FreeNode *ADDR_BEGIN;
extern FreeNode *ADDR_END;

int InitMyMalloc(int HeapSize);

void *MyMalloc(int size, int strategy);

int MyFree(void *ptr);

void DumpFreeList();

#endif
