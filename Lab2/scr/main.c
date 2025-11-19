#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "hex.h"

typedef struct {
    char **values;
    uint64_t total;
    char *partial_result;
    uint64_t processed_count;
} ThreadContext;

void* compute_partial_sum(void* context_ptr) {
    ThreadContext* ctx = (ThreadContext*)context_ptr;
    char* local_sum = strdup("0");
    uint64_t local_counter = 0;

    for (uint64_t idx = 0; idx < ctx->total; ++idx) {
        char* updated_sum = hex_add(local_sum, ctx->values[idx]);
        free(local_sum);
        local_sum = updated_sum;
        ++local_counter;
    }

    ctx->partial_result = local_sum;
    ctx->processed_count = local_counter;

    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        char usage_msg[] = "Usage: ./program <thread_count> <memory_size> <input_file>\n";
        write(STDOUT_FILENO, usage_msg, sizeof(usage_msg) - 1);
        return 1;
    }

    struct timespec begin_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &begin_time);

    uint64_t num_threads = strtoull(argv[1], NULL, 10);
    uint64_t memory_size = strtoull(argv[2], NULL, 10);

    FILE* input_file = fopen(argv[3], "r");
    if (!input_file) {
        char error_msg[] = "Failed to open input file\n";
        write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
        exit(EXIT_FAILURE);
    }

    uint64_t bytes_per_entry = 32 + 1 + 8;
    uint64_t max_entries_per_batch = memory_size / bytes_per_entry;
    if (max_entries_per_batch == 0) max_entries_per_batch = 1;

    char input_buffer[64];
    char** number_array = malloc(sizeof(char*) * max_entries_per_batch);

    char* overall_sum = strdup("0");
    uint64_t total_numbers = 0;

    pthread_t* thread_pool = malloc(sizeof(pthread_t) * num_threads);
    ThreadContext* thread_contexts = malloc(sizeof(ThreadContext) * num_threads);

    while (1) {
        uint64_t current_batch_size = 0;

        while (current_batch_size < max_entries_per_batch && 
               fgets(input_buffer, sizeof(input_buffer), input_file)) {
            char* newline_pos = input_buffer;
            while (*newline_pos && *newline_pos != '\n') ++newline_pos;
            *newline_pos = '\0';

            if (strlen(input_buffer) > 0) {
                number_array[current_batch_size] = strdup(input_buffer);
                ++current_batch_size;
            }
        }

        if (current_batch_size == 0) break;

        uint64_t base_per_thread = current_batch_size / num_threads;
        uint64_t extra_items = current_batch_size % num_threads;

        for (uint64_t t = 0; t < num_threads; ++t) {
            uint64_t start_index = t * base_per_thread + (t > 0 ? extra_items : 0);
            uint64_t item_count = base_per_thread + (t == 0 ? extra_items : 0);

            if (item_count > 0) {
                thread_contexts[t].values = number_array + start_index;
                thread_contexts[t].total = item_count;
                thread_contexts[t].partial_result = NULL;
                thread_contexts[t].processed_count = 0;

                pthread_create(&thread_pool[t], NULL, compute_partial_sum, &thread_contexts[t]);
            }
        }

        for (uint64_t t = 0; t < num_threads; ++t) {
            uint64_t item_count = base_per_thread + (t == 0 ? extra_items : 0);
            if (item_count > 0) {
                pthread_join(thread_pool[t], NULL);
            }
        }

        for (uint64_t t = 0; t < num_threads; ++t) {
            uint64_t item_count = base_per_thread + (t == 0 ? extra_items : 0);
            
            if (item_count > 0 && thread_contexts[t].partial_result) {
                char* new_total = hex_add(overall_sum, thread_contexts[t].partial_result);
                free(overall_sum);
                overall_sum = new_total;
                free(thread_contexts[t].partial_result);
                total_numbers += thread_contexts[t].processed_count;
            }
        }

        for (uint64_t i = 0; i < current_batch_size; ++i) {
            free(number_array[i]);
        }
    }

    char count_msg[64];
    snprintf(count_msg, sizeof(count_msg), "Total numbers processed: %lu\n", total_numbers);
    write(STDOUT_FILENO, count_msg, strlen(count_msg));

    char sum_msg[64];
    snprintf(sum_msg, sizeof(sum_msg), "Cumulative sum: %s\n", overall_sum);
    write(STDOUT_FILENO, sum_msg, strlen(sum_msg));

    if (total_numbers > 0) {
        char* mean_value = hex_div(overall_sum, total_numbers);
        char avg_msg[64];
        snprintf(avg_msg, sizeof(avg_msg), "Floored hexadecimal average: %s\n", mean_value);
        write(STDOUT_FILENO, avg_msg, strlen(avg_msg));
        free(mean_value);
    }

    free(number_array);
    free(overall_sum);
    free(thread_pool);
    free(thread_contexts);
    fclose(input_file);

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double time_elapsed = (end_time.tv_sec - begin_time.tv_sec) + (end_time.tv_nsec - begin_time.tv_nsec) / 1e9;
    
    char time_msg[64];
    snprintf(time_msg, sizeof(time_msg), "\nTotal execution time: %f seconds\n", time_elapsed);
    write(STDOUT_FILENO, time_msg, strlen(time_msg));
    
    char thread_msg[64];
    snprintf(thread_msg, sizeof(thread_msg), "Threads utilized: %lu\n", num_threads);
    write(STDOUT_FILENO, thread_msg, strlen(thread_msg));

    return 0;
}