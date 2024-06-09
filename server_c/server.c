#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include "blg312e.h"
#include "request.h"

// Define flags for scheduling algorithms
#define FIFO (1 << 0)
#define RFF (1 << 1)
#define SFF (1 << 2)

#define MAX_LINE (2048)

typedef struct {
    int* buf; // Array of file descriptors
    int size; // The size of the buf
    int fill_ptr; // queue tail ptr
    int use_ptr; // queue head ptr
    char **reqbuf; // store http requests
} FIFO_Buffer;

typedef struct {
    int* buf; // Array of file descriptors
    int* filesizes; // Size of the requested file (in seconds)
    int size; // The size of the buf
    char** reqbuf; // store http requests
} SFF_Buffer;

typedef struct {
    int* buf; // Array of file descriptors
    int* times; // Last modification time of the requested file (in seconds)
    int size; // The size of the buf
    char** reqbuf; // store http requests
} RFF_Buffer;

int strategy; // Choose between FIFO, SFF, RFF
sem_t empty; // Empty semaphore
sem_t full; // Full semaphore
sem_t mutex; // Binary semaphore

/*
* Read the HTTP request, then calculate statistics on the requested file
* @param fd  socket descriptor
* @param linebuf  store HTTP request string
* @param sbuf  store statistics of the file
*/
void getFileInfo(int fd, char *linebuf, struct stat *sbuf) {
    rio_t rio;
    
    char method[MAX_LINE], uri[MAX_LINE], version[MAX_LINE];
    char fname[MAX_LINE], cgiargs[MAX_LINE];
    
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, linebuf, MAXLINE);  // Read the http request
    
    sscanf(linebuf, "%s %s %s", method, uri, version);
    int is_static = requestParseURI(uri, fname, cgiargs);  // Extract file name
    printf("%s\n", fname);
    
    if (sbuf != NULL)
        Stat(fname, sbuf); // calculate statistics
    
    requestReadhdrs(&rio); // consume the leftover
}

/*
* Put the incoming request into the buffer using the previously chosen strategy
* @param buffer  place the incoming request
* @param value  the socket descriptor of the request
*/
void put(void* buffer, int value) {
    if (strategy & FIFO) {
        FIFO_Buffer *b = (FIFO_Buffer*) buffer; // cast the buffer
        char linebuf[MAX_LINE];
        getFileInfo(value, linebuf, NULL);  // do preprocessing on the file
        
        sem_wait(&mutex); // lock the critical section
        b->buf[b->fill_ptr] = value; // place the descriptor to the back of the queue
        strcpy(b->reqbuf[b->fill_ptr], linebuf); // store the http request
        b->fill_ptr = (b->fill_ptr + 1) % b->size; // advance the tail ptr
        sem_post(&mutex); // unlock
    } else if (strategy & RFF) {
        RFF_Buffer *b = (RFF_Buffer*) buffer;
        char linebuf[MAX_LINE];
        struct stat sbuf; // store file statictics
        getFileInfo(value, linebuf, &sbuf);
        
        sem_wait(&mutex); // lock the critical section
        // search an unoccupied spot in the buffer
        for (int i = 0; i < b->size; i++) {
            if (b->times[i] == -1) { // if the slot is empty
                b->buf[i] = value; // place the descriptor 
                b->times[i] = sbuf.st_mtim.tv_sec; // store the last modification time
                strcpy(b->reqbuf[i], linebuf);
                break;
            }
        }
        
        for (int i = 0; i < b->size; i++) 
        printf(" %d", b->times[i]);
        printf("\n");
        sem_post(&mutex); // unlock
    } else if (strategy & SFF) {
        SFF_Buffer *b = (SFF_Buffer*) buffer;
        char linebuf[MAX_LINE];
        struct stat sbuf;
        getFileInfo(value, linebuf, &sbuf);
        
        sem_wait(&mutex); // lock the critical section
        // search an unoccupied spot in the buffer
        for (int i = 0; i < b->size; i++) {
            if (b->filesizes[i] == -1) { // if the slot is empty
                b->buf[i] = value;
                b->filesizes[i] = sbuf.st_size; // store the file size
                strcpy(b->reqbuf[i], linebuf);
                break;
            }
        }
        
        for (int i = 0; i < b->size; i++) 
        printf(" %d", b->filesizes[i]);
        printf("\n");
        sem_post(&mutex); // unlock
    }
}

/*
* Get a suitable request from the buffer
* @param buffer  A buffer of requests
* @param buf  store the returned http request string
* @return the descriptor of the popped request 
*/
int get(void* buffer, char *buf) {
    int tmp; // output descriptor
    if (strategy & FIFO) {
        FIFO_Buffer *b = (FIFO_Buffer*) buffer;
        sem_wait(&mutex); // lock the critical section
        tmp = b->buf[b->use_ptr]; // pop from head
        b->buf[b->use_ptr] = -1; // mark as empty
        strcpy(buf, b->reqbuf[b->use_ptr]); // store the request
        b->use_ptr = (b->use_ptr + 1) % b->size; // update the head ptr
        sem_post(&mutex); // unlock
    } else if (strategy & RFF) {
        RFF_Buffer *b = (RFF_Buffer*) buffer;
        int maxtime = 0, maxindex = 0;
        sem_wait(&mutex); // lock the critical section
        // search for the recently modified file
        for (int i = 0; i < b->size; i++) {
            if (b->times[i] >= 0 && b->times[i] > maxtime) {
                maxtime = b->times[i];
                maxindex = i;
            }
        }
        tmp = b->buf[maxindex]; // remove from the buffer
        b->times[maxindex] = -1; // mark as empty
        strcpy(buf, b->reqbuf[maxindex]);
        
        for (int i = 0; i < b->size; i++) 
        printf(" %d", b->times[i]);
        printf("\n");
        sem_post(&mutex); // unlock
    } else if (strategy & SFF) {
        SFF_Buffer *b = (SFF_Buffer*) buffer;
        int minsize = ~(1 << 31), minindex = 0;
        sem_wait(&mutex); // lock the critical section
        // search for the file having the minimum size
        for (int i = 0; i < b->size; i++) {
            if (b->filesizes[i] >= 0 && b->filesizes[i] < minsize) {
                minsize = b->filesizes[i];
                minindex = i;
            }
        }
        tmp = b->buf[minindex]; // remove from the queue
        b->filesizes[minindex] = -1; // mark as empty
        strcpy(buf, b->reqbuf[minindex]);
        
        for (int i = 0; i < b->size; i++) 
        printf(" %d", b->filesizes[i]);
        printf("\n");
        sem_post(&mutex); // unlock
    }
    return tmp;
}

/*
* Worker function the consume requests from the buffer indefinitely
* @param arg  input buffer
*/
void *consumer(void *arg) {
    while (1) {
        char buf[MAX_LINE]; // http request string
        sem_wait(&full); // sleep if the buffer is empty
        int connfd = get(arg, buf); // get a request
        printf("consume: %d\n\n", connfd);
        sem_post(&empty); // increment empty semaphore
        if (connfd > 0) {
            requestHandle(connfd, buf);
		    Close(connfd);
		}
    }
}

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// blg312e: Parse the new arguments too
void getargs(int *port, int *threads, int *buffers, int *strategy, int argc, char *argv[])
{
    if (argc != 5) {
		fprintf(stderr, "Usage: %s <port> <threads> <buffers> <strategy>\n", argv[0]);
		exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
    if (strcmp(argv[4], "FIFO") == 0)
        *strategy = FIFO;
    else if (strcmp(argv[4], "RFF") == 0)
        *strategy = RFF;
    else if (strcmp(argv[4], "SFF") == 0)
        *strategy = SFF;
    else {
        fprintf(stderr, "Invalid strategy!\n");
        exit(1);
    }
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    int threads, buffers;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads, &buffers, &strategy, argc, argv);

    // 
    // blg312e: Create some threads...
    //
    void *buffer = NULL; // the buffer to be sent to threads
    // declare 3 buffers but initialize only one of them
    FIFO_Buffer fifo;
    SFF_Buffer sff;
    RFF_Buffer rff;
    if (strategy & FIFO) {
        fifo.buf = (int*) Malloc(buffers * sizeof(int));
        fifo.reqbuf = (char**) Malloc(buffers * sizeof(char*));
        fifo.size = buffers;
        fifo.fill_ptr = 0;
        fifo.use_ptr = 0;
        
        for (int i = 0; i < fifo.size; i++) {
            fifo.buf[i] = -1;
            fifo.reqbuf[i] = (char*) Malloc(MAX_LINE * sizeof(char));
        }
            
        buffer = (void*) (&fifo); // buffer is FIFO
    } else if (strategy & SFF) {
        sff.buf = (int*) Malloc(buffers * sizeof(int));
        sff.filesizes = (int*) Malloc(buffers * sizeof(int));
        sff.reqbuf = (char**) Malloc(buffers * sizeof(char*));
        sff.size = buffers;
        
        for (int i = 0; i < sff.size; i++) {
            sff.buf[i] = -1;
            sff.filesizes[i] = -1;
            sff.reqbuf[i] = (char*) Malloc(MAX_LINE * sizeof(char));
        }
            
        buffer = (void*) (&sff); // buffer is SFF
    } else if (strategy & RFF) {
        rff.buf = (int*) Malloc(buffers * sizeof(int));
        rff.times = (int*) Malloc(buffers * sizeof(int));
        rff.reqbuf = (char**) Malloc(buffers * sizeof(char*));
        rff.size = buffers;
        
        for (int i = 0; i < rff.size; i++) {
            rff.buf[i] = -1;
            rff.times[i] = -1;
            rff.reqbuf[i] = (char*) Malloc(MAX_LINE * sizeof(char));
        }
            
        buffer = (void*) (&rff); // buffer is RFF
    }
    
    // Initialize the semaphores
    sem_init(&empty, 0, buffers);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    
    // Create threads
    pthread_t *ts = (pthread_t*) Malloc(threads * sizeof(pthread_t));
    for (int i = 0; i < threads; ++i)
        Pthread_create(&ts[i], NULL, &consumer, buffer);
    
    listenfd = Open_listenfd(port);
    while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		
		// 
		// blg312e: In general, don't handle the request in the main thread.
		// Save the relevant info in a buffer and have one of the worker threads 
		// do the work.
		// 
		
        sem_wait(&empty); // sleep until the buffer is not empty
        put(buffer, connfd); // place the request into the buffer
        printf("produce: %d\n\n", connfd); 
        sem_post(&full); // increment the full semaphore
    }
}


    


 
