#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "list.h"

#define MAX_LINE_LEN 256
#define BUFFER_SIZE 1024

void collect(char *cmdstr, const char *delim, clist_t list);
void exe_cmd(char **cmd, int **fds, size_t order, size_t capacity, clist_t hist);
int exe_line(char *line, clist_t hist);
int exe_file(char *filename, clist_t hist);
void exe_hist(clist_t hist);
void print_cmd(char **cmd);

int main() {
    clist_t hist = construct_list();
for (;;) {
    char input[MAX_LINE_LEN];
    char cwd[MAX_LINE_LEN];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "getcwd() error.\n");
        exit(EXIT_FAILURE);
    }
    printf("%s$ ", cwd);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';
    
    int empty_flag = 0;
    for (const char *ptr = input; *ptr != '\0'; ptr++) 
        if (*ptr != ' ')
            empty_flag = 1;
    if (empty_flag == 0)
        continue;
    
    int exitflag = 0;
    if (strstr(input, "bash") != NULL) {
        push(hist, input, strcspn(input, "\0") + 1, size(hist));
        clist_t args = construct_list();
        collect(input, " ", args);
        for (size_t i = 0; i < size(args); i++) {
            if (strstr((char*) get_from(args, i), ".sh") != NULL)
                exitflag = exe_file((char*) get_from(args, i), hist);
        }
        destruct(&args);
    } else {
        exitflag = exe_line(input, hist);
    }
    if (exitflag) 
        break;
}
    destruct(&hist);
    return EXIT_SUCCESS;
}

void collect(char *cmdstr, const char *delim, clist_t list) {
    char *token, *saveptr = NULL;
    int i = 0;
    for (token = strtok_r(cmdstr, delim, &saveptr);
         token != NULL;
         token = strtok_r(NULL, delim, &saveptr)) {
        push(list, token, (strlen(token) + 1) * sizeof(char), size(list));
        i++;
    }
}

void exe_cmd(char **cmd, int **fds, size_t order, size_t capacity, clist_t hist) {
    if (strcmp(cmd[0], "quit") == 0) 
        return;
    if (strcmp(cmd[0], "history") == 0) {
        exe_hist(hist);
        return;
    } 
    if (strcmp(cmd[0], "cd") == 0) {
        if(chdir(cmd[1]) < 0)
            fprintf(stderr, "Change directory error.\n");
        return;
    }
    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "Fork failed.\n");
        exit(EXIT_FAILURE);
    } else if (child == 0) {
        print_cmd(cmd);
        if (order > 0) {
            if (dup2(fds[order - 1][0], STDIN_FILENO) < 0) {
                fprintf(stderr, "Dup2 failed (%ld).\n", order - 1);
                exit(EXIT_FAILURE);
            }
        }
        if (order < capacity) {
            if (dup2(fds[order][1], STDOUT_FILENO) < 0) {
                fprintf(stderr, "Dup2 failed (%ld).\n", order);
                exit(EXIT_FAILURE);
            }
        }
        for (size_t i = 0; i < capacity; i++) {
            close(fds[i][0]);
            close(fds[i][1]);
        }
        execvp(cmd[0], cmd);
        fprintf(stderr, "Execution failed.\n");
        exit(EXIT_SUCCESS);
    } else {
    }
}

int exe_line(char *line, clist_t hist) {
    int exitflag = 0;

    push(hist, line, strcspn(line, "\0") + 1, size(hist));
    size_t process_count = 0;
    clist_t pipes = construct_list();
    collect(line, "|", pipes);
    // pipe start
    int **pip = (int**) malloc((size(pipes) - 1) * sizeof(int*));
    for (size_t i = 0; i < size(pipes) - 1; i++) {
        pip[i] = (int*) malloc(2 * sizeof(int));
        if (pipe(pip[i]) < 0) {
            fprintf(stderr, "Pipe failed (%ld).\n", i);
            exit(EXIT_FAILURE);
        }
    }
    // pipe end
    for (size_t i = 0; i < size(pipes); i++) {
        clist_t scols = construct_list();
        collect((char*) get_from(pipes, i), ";", scols);
        process_count += size(scols);
        for (size_t j = 0; j < size(scols); j++) {
            clist_t spaces = construct_list();
            char *semicmd = (char*) get_from(scols, j);
            collect(semicmd, " ", spaces);
            char **exein = (char**) malloc((size(spaces) + 1) * sizeof(char*));
            for (size_t k = 0; k < size(spaces); k++) {
                if (strcmp((char*) get_from(spaces, k), "quit") == 0)
                    exitflag = 1;
                exein[k] = (char*) get_from(spaces, k);
            }           
            // execute start
            exe_cmd(exein, pip, i, size(pipes) - 1, hist);
            // execute end
            free(exein);
            destruct(&spaces);
        }
        destruct(&scols);
    }
    
    for (size_t i = 0; i < size(pipes) - 1; i++) {
        close(pip[i][0]);
        close(pip[i][1]);
        free(pip[i]);
    }
    
    for (size_t i = 0; i < process_count; i++) {
        int status;
        wait(&status); 
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Child process failed with exit status %d\n", WEXITSTATUS(status));
            if (size(pipes) - 1 > 0)
                free(pip);
            destruct(&pipes);
            
            return exitflag;
        }
    }
    if (size(pipes) - 1 > 0)
        free(pip);
    destruct(&pipes);
    
    return exitflag;
}

int exe_file(char *filename, clist_t hist) {
    int exitflag = 0;
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) {
        fprintf(stderr, "Could not open the file \"%s\".\n", filename);
        return exitflag;
    }
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fptr) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        exitflag = exe_line(line, hist);
        if (exitflag)
            break;
    }
    fclose(fptr);
    return exitflag;
}


void exe_hist(clist_t hist) {
    for (size_t i = 0; i < size(hist); i++) {
        printf(" %ld %s \n", i, (char*) get_from(hist, i));
    }
}

void print_cmd(char **cmd) {
    printf("Running command: ");
    for (size_t i = 0; cmd[i] != NULL; i++)
        printf("%s ", cmd[i]);
    printf("\n");
}
