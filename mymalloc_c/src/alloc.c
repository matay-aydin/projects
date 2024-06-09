#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "alloc.h"

int InitCalled = 0;
unsigned long int HEAP_SIZE = 0;
FreeNode *FreeHead = NULL;
FreeNode *ADDR_BEGIN = NULL; // Top of the heap
FreeNode *ADDR_END = NULL; // Bottom of the heap

int InitMyMalloc(int HeapSize) {
    if (InitCalled == 1 || HeapSize <= 0)
        return -1; // fail
    // Calculate the total size needed to accommodate both FreeNode and user data
    size_t totalSize;
    if (HeapSize % getpagesize() == 0)
        totalSize = HeapSize;
    else // round up
        totalSize = (HeapSize / getpagesize() + 1) * getpagesize();

    // Align the total size to ensure proper memory allocation
    size_t alignedSize = (totalSize + sizeof(FreeNode) - 1) & ~(sizeof(FreeNode) - 1);
    
    FreeHead = mmap(NULL, alignedSize, 
                    PROT_READ|PROT_WRITE, 
                    MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    if (FreeHead == MAP_FAILED) {
        perror("mmap");
        return -1; // fail
    }
    // Initialize the head ptr
    FreeHead->size = 0;
    FreeHead->next = FreeHead + 2;
    
    // Initialize the next (fit) node
    (FreeHead + 1)->size = 0;
    (FreeHead + 1)->next = FreeHead + 2;
    
    // Initialize the head node
    (FreeHead + 2)->size = alignedSize - sizeof(Header) - 2 * sizeof(FreeNode);
    (FreeHead + 2)->next = NULL;
    
    // Initialize the global variables
    HEAP_SIZE = alignedSize - 2 * sizeof(FreeNode);
    ADDR_BEGIN = FreeHead + 2;
    ADDR_END = (FreeNode*) ((char*) FreeHead + alignedSize);
    InitCalled = 1;
    
    return 0; // success
}

void *MyMalloc(int size, int strategy) {
    FreeNode *freefound = FreeHead->next, *freeprev = NULL;
    if (size <= 0) return NULL;
    if (strategy == BF) { // best fit
        int min_size = ~(1 << (8 * sizeof(int) - 1)); // int max
        for (;;) {
            if (freefound == NULL)
                break;
            if (freefound->size >= size && freefound->size < min_size) // a free block is found
                min_size = freefound->size;
            freefound = freefound->next;
        }
        freefound = FreeHead->next;
        for (;;) {
            if (freefound == NULL) // no heap for this request
                return NULL;
            if (freefound->size == min_size) // the best fit is found
                break;
            freeprev = freefound; // previous free block is saved
            freefound = freefound->next;
        }
    } else if (strategy == WF) { // worst fit
        int max_size = (1 << (8 * sizeof(int) - 1)); // int min
        for (;;) {
            if (freefound == NULL)
                break;
            if (freefound->size >= size && freefound->size > max_size) // a free block is found
                max_size = freefound->size;
            freefound = freefound->next;
        }
        freefound = FreeHead->next;
        for (;;) {
            if (freefound == NULL) // no heap for this request
                return NULL;
            if (freefound->size == max_size) // the best fit is found
                break;
            freeprev = freefound; // previous free block is saved
            freefound = freefound->next;
        }
    } else if (strategy == FF) { // first fit
        for (;;) {
            if (freefound == NULL) // no heap for this request
                return NULL;
            if (freefound->size >= size) // a free block is found
                break;
            freeprev = freefound; // previous free block is saved
            freefound = freefound->next;
        }
    } else if (strategy == NF) { // next fit
        if ((FreeHead + 1)->next == NULL) // reset the next pointer
            (FreeHead + 1)->next = FreeHead->next;
        for (;;) {
            if ((FreeHead + 1)->next == NULL) // no heap for this request
                return NULL;
            if ((FreeHead + 1)->next->size >= size) // a free block is found
                break;
            freeprev = (FreeHead + 1)->next; // previous free block is saved
            (FreeHead + 1)->next = (FreeHead + 1)->next->next; 
        }
        freefound = (FreeHead + 1)->next;
        (FreeHead + 1)->next = (FreeHead + 1)->next->next; // advance to a valid node;
    } else return NULL; // invalid strategy
    
    // Adjust the next (fit) pointer
    if (freefound == (FreeHead + 1)->next && strategy != NF) 
        (FreeHead + 1)->next = (FreeHead)->next->next;
    
    // Check if there would left place to fit the remaining node
    if (freefound->size > size + (int) sizeof(FreeNode)) {
        if (freefound == FreeHead->next) {
            FreeHead->next = (FreeNode*) ((char*) freefound + size + sizeof(Header)); // Update the position
            FreeHead->next->size = freefound->size - size - sizeof(Header); // Decrease the free size
            FreeHead->next->next = freefound->next;
        } else {
            freeprev->next = (FreeNode*) ((char*) freefound + size + sizeof(Header)); // Update the position
            freeprev->next->size = freefound->size - size - sizeof(Header); // Decrease the free size
            freeprev->next->next = freefound->next;
        }
    } else { // Entire node should be allocated
        size = freefound->size;
        if (freefound == FreeHead->next) {
            if (FreeHead->next->next == NULL) // No place would left to fit the head node
                return NULL;
            FreeHead->next = FreeHead->next->next;
        } else {
            // freefound != FreeHead->next implies that it has a prev node
            freeprev->next = freefound->next;
        }
    }
    
    // Initialize the header of the allocated block
    Header *hptr = (Header*) freefound; 
    hptr->size = size;
    hptr->magic = MAGIC;
    // Skip the header and return
    return (void*) ((char*) hptr + sizeof(Header));
}

int MyFree(void *freed) {
    if (freed == NULL) return -1;
    // Move back to the header
    Header *freeheader = (Header*) ((char*) freed - sizeof(Header));
    
    // Do an integrity check
    if (freeheader->magic != MAGIC) return -1;
    
    // Append the new node to the head
    FreeNode *temp = (FreeNode*) freeheader;
    temp->size = freeheader->size;
    temp->next = FreeHead->next;
    FreeHead->next = temp;
    
    // Coalesce with prev block
    // Find the upper adjacent free block
    for (FreeNode *ptr = FreeHead->next;;) {
        if (ptr->next == NULL) break;
        if (FreeHead->next == (FreeNode*) ((char*) ptr->next + ptr->next->size + sizeof(Header))) {
            ptr->next->size += FreeHead->next->size + sizeof(Header); // Assign coalesced node size
            FreeNode *tempnext = ptr->next;
            if (ptr != FreeHead->next) {
                FreeNode *tempnextnext = ptr->next->next; // Save addresses of next and nextnext node
                ptr->next->next = FreeHead->next->next; // Save old head's next node address
                ptr->next = tempnextnext; // Save prev block's next node address
            }
            FreeHead->next = tempnext; // Move then head of the list to the beginning of the coalesced block
            break;
        }
        ptr = ptr->next;
    }
    
    // Coalesce with next block
    // Find the lower adjacent free block
    for (FreeNode *ptr = FreeHead->next;;) {
        if (ptr->next == NULL) break;
        if (ptr->next == (FreeNode*) ((char*) FreeHead->next + FreeHead->next->size + sizeof(Header))) {
            FreeHead->next->size += ptr->next->size + sizeof(Header);
            if (ptr == FreeHead->next)
                FreeHead->next->next = FreeHead->next->next->next;
            else    
                ptr->next = ptr->next->next;
            break;
        }
        ptr = ptr->next;
    }
    
    return 0;
}

void DumpFreeList() {
    printf("\n%10s %16s %16s %10s\n", "Block", "Address", "Size", "Status");
    int addr = 0, block = 1, blocksAlloc = 0, blocksFree = 0;
    unsigned long int bytesAlloc = 0, bytesFree = 0;
    for (FreeNode *ptr = ADDR_BEGIN;; block++) { // Iterate over all blocks
        if (ptr == ADDR_END) break; // End Of Heap
        Header *hptr = (Header*) ptr;
        if (hptr->magic == MAGIC) { // Check if the current block is a header (allocated)
            bytesAlloc += sizeof(Header) + hptr->size;
            blocksAlloc++;
            printf("%10d %16d %16d %10s\n", block, addr, hptr->size, "Alloc\'d");
            addr += sizeof(Header) + hptr->size; // Calc. the relative addr. of the next block
            ptr = (FreeNode*) ((char*) hptr + sizeof(Header) + hptr->size); // Move to the next block
        } else { // The block is a free node
            bytesFree += sizeof(Header) + ptr->size;
            blocksFree++;
            printf("%10d %16d %16d %10s\n", block, addr, ptr->size, "Free");
            addr += sizeof(Header) + ptr->size; // Calc. the relative addr. of the next block
            ptr = (FreeNode*) ((char*) ptr + sizeof(Header) + ptr->size); // Move to the next block
        }
    }
    printf("%34s\n", "Heap Summary");
    printf("%27s %16d\n", "Allocated blocks:", blocksAlloc);
    printf("%27s %16lu\n", "Allocated bytes:", bytesAlloc);
    printf("%27s %16lu\n", "Bytes in use:", bytesAlloc - blocksAlloc * sizeof(Header));
    printf("%27s %16d\n", "Free blocks:", blocksFree);
    printf("%27s %16lu\n", "Free bytes:", bytesFree);
    printf("%27s %16lu\n\n", "Bytes can be used:", bytesFree - blocksFree * sizeof(Header));
}

