#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "alloc.h"

// helper function
char* alloc(int size, int strategy) {
    char *a = (char*) MyMalloc(size * sizeof(char), strategy);
    if (a == NULL) {
        fprintf(stderr, "MyMalloc has returned NULL.\n");
        return NULL;
    }
    return a;
}

int main() {
    if (InitMyMalloc(4096) == -1) {
        fprintf(stderr, "Initialization failed.\n");
        exit(EXIT_FAILURE);
    }
    
    DumpFreeList(); // Before allocation
    
    const int SIZE = 10;
    char *arr[SIZE];
    for (int i = 0; i < SIZE; ++i) 
        arr[i] = alloc((i + 1) * 10, FF);
        
    // Allocate a block to store the pointers created in the processes
    // to free them later on
    const int PROC = 4;
    char **blocks = (char**) MyMalloc(PROC * sizeof(char*), WF);
    if (blocks == NULL) {
        fprintf(stderr, "MyMalloc has returned NULL.\n");
        exit(EXIT_FAILURE);
    };
    
    DumpFreeList(); // After allocation of 11 blocks
    
    // Free some blocks to enlarge the free list
    for (int i = 0; i < SIZE; i += 2) {
        if (MyFree(arr[i]) == -1) {
            fprintf(stderr, "Free failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    DumpFreeList(); // After deallocation of some blocks

    // Fork child processes
    for (int i = 0; i < PROC; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            
            int strategy = 0;
            printf("Enter strategy [0 - 3]: ");
            scanf("%d", &strategy);
            if (strategy < 0 || strategy > 3)
                strategy = 0;
            
            printf("Current strategy is: ");
            switch (strategy) {
                case 0:
                    printf("Best Fit\n");
                    break;
                case 1:
                    printf("Worst Fit\n");
                    break;
                case 2:
                    printf("First Fit\n");
                    break;
                case 3:
                    printf("Next Fit\n");
                    break;
            }
            
            int allocSize = 0;
            printf("Enter the size of the memory block: ");
            scanf("%d", &allocSize);
            if (allocSize <= 0)
                allocSize = 100;
            
            blocks[i] = alloc(allocSize, strategy);
            
            printf("The block is successfully allocated!\n");
            printf("%27s %16d\n", "Process ID:", getpid());
            printf("%27s %16p\n", "Process Heap Address:", blocks[i]);
            DumpFreeList();
            
            exit(EXIT_SUCCESS);
        } else { // Main process
            wait(NULL); // Wait for the child process to finish
        }
    }

    for (int i = 1; i < SIZE; i += 2) {
        if (MyFree(arr[i]) == -1) {
            fprintf(stderr, "Free failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < PROC; i++) {
        if (MyFree(blocks[i]) == -1) {
            fprintf(stderr, "Free failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    if (MyFree(blocks) == -1) {
        fprintf(stderr, "Free failed.\n");
        exit(EXIT_FAILURE);
    }
    
    DumpFreeList(); // After cleanup
            
    munmap(ADDR_BEGIN, HEAP_SIZE); // unmap the heap
    return EXIT_SUCCESS;
}
