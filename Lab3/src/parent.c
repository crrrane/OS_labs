#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#define SHM_SIZE 4096

typedef struct {
    char filename[256];
    char numbers[1024];
    char result[256];
    int has_data;
    int division_by_zero;
    int shutdown;
} shared_data_t;

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <unique_id>\n", argv[0]);
        return 1;
    }

    char shm_name[256], sem_name[256];
    snprintf(shm_name, sizeof(shm_name), "/lab_shm_%s", argv[1]);
    snprintf(sem_name, sizeof(sem_name), "/lab_sem_%s", argv[1]);

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    shared_data_t *shared_data = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    memset(shared_data, 0, sizeof(shared_data_t));

    sem_t *semaphore = sem_open(sem_name, O_CREAT, 0666, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open failed");
        return 1;
    }

    printf("Server started with ID: %s\n", argv[1]);
    printf("Shared memory: %s\n", shm_name);
    printf("Semaphore: %s\n", sem_name);
    printf("Waiting for client...\n");

    printf("Enter filename: ");
    char filename[256];
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("Error reading filename\n");
        return 1;
    }
    filename[strcspn(filename, "\n")] = '\0';

    sem_wait(semaphore);
    strcpy(shared_data->filename, filename);
    sem_post(semaphore);

    printf("Enter numbers separated by spaces (e.g., '12 3 4'):\n");
    printf("Press Enter on empty line to exit\n");

    char input[1024];
    while (fgets(input, sizeof(input), stdin)) {
        if (input[0] == '\n') {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        sem_wait(semaphore);
        strcpy(shared_data->numbers, input);
        shared_data->has_data = 1;
        shared_data->division_by_zero = 0;
        memset(shared_data->result, 0, sizeof(shared_data->result));
        sem_post(semaphore);

        int processed = 0;
        while (!processed) {
            sem_wait(semaphore);
            if (!shared_data->has_data) {
                processed = 1;
                if (shared_data->division_by_zero) {
                    printf("Error: Division by zero! Exiting...\n");
                    sem_post(semaphore);

                    munmap(shared_data, SHM_SIZE);
                    close(shm_fd);
                    shm_unlink(shm_name);
                    sem_close(semaphore);
                    sem_unlink(sem_name);
                    return 1;
                }
            }
            sem_post(semaphore);
            usleep(100000);
        }

        sem_wait(semaphore);
        if (strlen(shared_data->result) > 0) {
            printf("Result: %s", shared_data->result);
        }
        sem_post(semaphore);
    }

    sem_wait(semaphore);
    shared_data->shutdown = 1;
    sem_post(semaphore);

    printf("Program finished successfully.\n");

    munmap(shared_data, SHM_SIZE);
    close(shm_fd);
    shm_unlink(shm_name);
    sem_close(semaphore);
    sem_unlink(sem_name);

    return 0;
}