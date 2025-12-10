#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define SHM_SIZE 4096

typedef struct {
    char filename[256];
    char numbers[1024];
    char result[256];
    int has_data;
    int division_by_zero;
    int shutdown;
} shared_data_t;

void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

int main(int argc, char **argv) {
    if (argc != 2) {
        write_str("Usage: ");
        write_str(argv[0]);
        write_str(" <unique_id>\n");
        return 1;
    }

    char shm_name[256], sem_name[256];
    snprintf(shm_name, sizeof(shm_name), "/lab_shm_%s", argv[1]);
    snprintf(sem_name, sizeof(sem_name), "/lab_sem_%s", argv[1]);

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        write_str("shm_open failed\n");
        return 1;
    }

    shared_data_t *shared_data = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        write_str("mmap failed\n");
        return 1;
    }

    sem_t *semaphore = sem_open(sem_name, 0);
    if (semaphore == SEM_FAILED) {
        write_str("sem_open failed\n");
        return 1;
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Client started with ID: %s\n", argv[1]);
    write_str(buffer);

    char filename[256];
    sem_wait(semaphore);
    strcpy(filename, shared_data->filename);
    sem_post(semaphore);

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        write_str("Error: failed to open file ");
        write_str(filename);
        write_str("\n");
        return 1;
    }

    snprintf(buffer, sizeof(buffer), "Output file: %s\n", filename);
    write_str(buffer);

    while (1) {
        sem_wait(semaphore);
        
        if (shared_data->shutdown) {
            sem_post(semaphore);
            break;
        }

        if (shared_data->has_data) {
            char numbers[1024];
            strcpy(numbers, shared_data->numbers);
            sem_post(semaphore);

            int nums[100];
            int count = 0;
            char *token = strtok(numbers, " \t");
            
            while (token != NULL && count < 100) {
                nums[count++] = atoi(token);
                token = strtok(NULL, " \t");
            }

            if (count < 2) {
                const char *error_msg = "Error: need at least 2 numbers\n";
                fprintf(file, "%s", error_msg);
                fflush(file);
                
                sem_wait(semaphore);
                strcpy(shared_data->result, error_msg);
                shared_data->has_data = 0;
                sem_post(semaphore);
                continue;
            }

            int division_by_zero = 0;
            for (int i = 1; i < count; i++) {
                if (nums[i] == 0) {
                    division_by_zero = 1;
                    break;
                }
            }
            
            if (division_by_zero) {
                const char *error = "Error: division by zero\n";
                fprintf(file, "%s", error);
                fflush(file);
                
                sem_wait(semaphore);
                shared_data->division_by_zero = 1;
                shared_data->has_data = 0;
                sem_post(semaphore);
                
                fclose(file);
                return 1;
            }

            float result = (float)nums[0];
            for (int i = 1; i < count; i++) {
                result /= nums[i];
            }

            fprintf(file, "%d", nums[0]);
            for (int i = 1; i < count; i++) {
                fprintf(file, " / %d", nums[i]);
            }
            fprintf(file, " = %.2f\n", result);
            fflush(file);

            char response[50];
            snprintf(response, sizeof(response), "%.2f\n", result);
            
            sem_wait(semaphore);
            strcpy(shared_data->result, response);
            shared_data->has_data = 0;
            sem_post(semaphore);
        } else {
            sem_post(semaphore);
            usleep(100000);
        }
    }

    fclose(file);
    write_str("Client finished successfully.\n");

    munmap(shared_data, SHM_SIZE);
    close(shm_fd);
    sem_close(semaphore);

    return 0;
}
