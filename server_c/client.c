/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      client www.cs.wisc.edu 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * blg312e: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "blg312e.h"

typedef struct {
    char* host;
    int port;
    char* filename;
} ReqArgs;

/*
* Worker routine of the threads
* @param arg  ReqArgs that stores the host info and the filename
*/
void *worker(void *arg) {
    ReqArgs *request = (ReqArgs*) arg;
    /* Open a single connection to the specified host and port */
    int clientfd = Open_clientfd(request->host, request->port);

    clientSend(clientfd, request->filename);
    clientPrint(clientfd);

    Close(clientfd);
    return NULL;
}

/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%s host: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
  
  printf("\n\n");
}

int main(int argc, char *argv[])
{
    // The input txt should contain the requests line by line
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "<filename>: Name of the file that contains the requests.\n");
        exit(1);
    }
    
    FILE *fp = NULL, *ctr = NULL;
    int lines = 0;
    char line[512];
    
    // Open the txt file
    ctr = Fopen(argv[1], "r");
    
    // Count the number of requests
    while (fgets(line, sizeof(line), ctr) != NULL)
        lines++;
    
    fclose(ctr);
    
    ReqArgs *reqs = (ReqArgs*) Malloc(lines * sizeof(ReqArgs));
    
    // Reopen the txt file
    fp = Fopen(argv[1], "r");
    
    // Extract tokens from the lines
    for (int i = 0; i < lines; i++) {
        if (fgets(line, sizeof(line), fp) == NULL)
            break;
            
        char *newline = strchr(line, '\n');
        if (newline)
          *newline = '\0';
            
        char *token = strtok(line, " ");
        reqs[i].host = (char*) malloc((strlen(token) + 1) * sizeof(char));
        strcpy(reqs[i].host, token);
        
        token = strtok(NULL, " ");
        reqs[i].port = atoi(token);
        
        token = strtok(NULL, " ");
        reqs[i].filename = (char*) malloc((strlen(token) + 1) * sizeof(char));
        strcpy(reqs[i].filename, token);
    }
    
    fclose(fp);
    
    // Create threads
    pthread_t *tis = (pthread_t*) malloc(lines * sizeof(pthread_t));
    for (int r = 0; r < lines; r++) 
        Pthread_create(&tis[r], NULL, worker, (void*) (reqs + r));
    
    // Wait for threads to complete
    for (int r = 0; r < lines; r++) 
        Pthread_join(tis[r], NULL);
        
    // Do cleanup
    free(tis);
    free(reqs);

    exit(0);
}
