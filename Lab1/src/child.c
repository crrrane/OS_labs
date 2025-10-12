#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char buf[4096];
    ssize_t bytes;

    char filename[256];
    if (read(STDIN_FILENO, filename, sizeof(filename)-1) <= 0) {
        const char msg[] = "Error: failed to read filename\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    filename[strcspn(filename, "\n")] = '\0';

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        const char msg[] = "Error: failed to open file\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    while ((bytes = read(STDIN_FILENO, buf, sizeof(buf)-1)) > 0) {
        buf[bytes] = '\0';

        int numbers[100];
        int count = 0;
        char *token = strtok(buf, " \t\n");
        
        while (token != NULL && count < 100) {
            numbers[count++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }

        if (count < 2) {
            const char error_msg[] = "Error: need at least 2 numbers\n";
            write(STDOUT_FILENO, error_msg, strlen(error_msg));
            write(file, error_msg, strlen(error_msg));
            continue;
        }

        int division_by_zero = 0;
        for (int i = 1; i < count; i++) {
            if (numbers[i] == 0) {
                division_by_zero = 1;
                break;
            }
        }
        
        if (division_by_zero) {
            const char error[] = "Error: division by zero\n";
            write(file, error, strlen(error));

            const char signal[] = "DIVISION_BY_ZERO\n";
            write(STDOUT_FILENO, signal, strlen(signal));
            
            close(file);
            exit(EXIT_FAILURE);
        }

        float result = (float)numbers[0];
        for (int i = 1; i < count; i++) {
            result /= numbers[i];
        }

        dprintf(file, "%d", numbers[0]);
        for (int i = 1; i < count; i++) {
            dprintf(file, " / %d", numbers[i]);
        }
        dprintf(file, " = %.2f\n", result);

        char response[50];
        snprintf(response, sizeof(response), "%.2f\n", result);
        write(STDOUT_FILENO, response, strlen(response));
    }
    
    close(file);
    return 0;
}