#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int parent_to_child[2];
    int child_to_parent[2];
    
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe failed");
        return 1;
    }

    const pid_t child = fork();

    switch (child) {
    case -1: {
        perror("fork failed");
        return 1;
    } break;

    case 0: {
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        
        dup2(parent_to_child[0], STDIN_FILENO);
        close(parent_to_child[0]);
        
        dup2(child_to_parent[1], STDOUT_FILENO);
        close(child_to_parent[1]);
        
        execl("./child", "child", NULL);
        perror("exec failed");
        return 1;
    } break;

    default: {
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        
        printf("Enter filename: ");
        char filename[256];
        if (fgets(filename, sizeof(filename), stdin) == NULL) {
            printf("Error reading filename\n");
            return 1;
        }
        filename[strcspn(filename, "\n")] = '\0';

        write(parent_to_child[1], filename, strlen(filename));
        write(parent_to_child[1], "\n", 1);
        
        printf("Enter numbers separated by spaces (e.g., '12 3 4'):\n");
        printf("Press Enter on empty line to exit\n");
        
        char input[1024];
        while (fgets(input, sizeof(input), stdin)) {
            if (input[0] == '\n') {
                break;
            }

            write(parent_to_child[1], input, strlen(input));
            
            char response[100];
            ssize_t bytes = read(child_to_parent[0], response, sizeof(response)-1);
            
            if (bytes > 0) {
                response[bytes] = '\0';
                
                if (strstr(response, "DIVISION_BY_ZERO") != NULL) {
                    printf("Error: Division by zero! Exiting...\n");
                    close(parent_to_child[1]);
                    close(child_to_parent[0]);
                    exit(EXIT_FAILURE);
                }
                
                printf("Result: %s", response);
            }
        }

        close(parent_to_child[1]);
        close(child_to_parent[0]);
        wait(NULL);
        printf("Program finished successfully.\n");
    } break;
    }
    
    return 0;
}