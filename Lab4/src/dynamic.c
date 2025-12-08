#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#define BUFFER_SIZE 1024

typedef float (*cos_derivative_func)(float, float);
typedef float (*area_func)(float, float);

typedef enum 
{
    SUCCESS = 0,
    LIBRARY_ERROR = -1,
} StatusCode;

typedef enum 
{
    LIB_VERSION_1 = 0,
    LIB_VERSION_2 = 1,
} LibraryVersion;

StatusCode switch_library(const char** lib_paths, void** lib_handle, LibraryVersion* current_version, cos_derivative_func* deriv_func, area_func* area_func_ptr)
{
    dlclose(*lib_handle);

    *current_version = (*current_version == LIB_VERSION_1) ? LIB_VERSION_2 : LIB_VERSION_1;

    char message[BUFFER_SIZE];

    *lib_handle = dlopen(lib_paths[*current_version], RTLD_LAZY);
    if (!(*lib_handle))
    {
        int msg_len = snprintf(message, BUFFER_SIZE, "Library loading error: %s\n", dlerror());
        write(STDERR_FILENO, message, msg_len);
        return LIBRARY_ERROR;
    }

    *deriv_func = dlsym(*lib_handle, "cos_derivative");
    *area_func_ptr = dlsym(*lib_handle, "area");

    if (!*deriv_func || !*area_func_ptr)
    {
        const char error_msg[] = "Error: failed to locate functions in library\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        return LIBRARY_ERROR;
    }

    {
        int msg_len = snprintf(message, BUFFER_SIZE, "Loaded library: %s\n", lib_paths[*current_version]);
        write(STDOUT_FILENO, message, msg_len);
    }

    return SUCCESS;
}

void execute_derivative(cos_derivative_func deriv_func)
{
    char* a_str = strtok(NULL, " \t\n");
    char* dx_str = strtok(NULL, " \t\n");
    
    char output[BUFFER_SIZE];

    if (a_str && dx_str)
    {
        float a_val = atof(a_str);
        float dx_val = atof(dx_str);
        float result = deriv_func(a_val, dx_val);
        
        int msg_len = snprintf(output, BUFFER_SIZE, "cos'(%.4f) = %.8f (dx=%.6f)\n", a_val, result, dx_val);
        write(STDOUT_FILENO, output, msg_len);
    }
    else
    {
        const char* error_msg = "Error: a and dx values required\n";
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
    }
}

void execute_area(area_func area_func_ptr)
{
    char* a_str = strtok(NULL, " \t\n");
    char* b_str = strtok(NULL, " \t\n");
    
    char output[BUFFER_SIZE];

    if (a_str && b_str)
    {
        float a_val = atof(a_str);
        float b_val = atof(b_str);
        float result = area_func_ptr(a_val, b_val);
        
        int msg_len = snprintf(output, BUFFER_SIZE, "Area with sides %.3f and %.3f = %.6f\n", a_val, b_val, result);
        write(STDOUT_FILENO, output, msg_len);
    }
    else
    {
        const char* error_msg = "Error: two side values required\n";
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
    }
}

int main()
{
    const char* library_files[] = {"./liblib1.so", "./liblib2.so"};
    LibraryVersion active_lib = LIB_VERSION_1;

    cos_derivative_func cos_derivative_func_ptr = NULL;
    area_func area_func_ptr = NULL;

    char message_buffer[BUFFER_SIZE];

    void* loaded_library = dlopen(library_files[active_lib], RTLD_LAZY);
    if (!loaded_library)
    {
        int msg_len = snprintf(message_buffer, BUFFER_SIZE, "Initialization error: %s\n", dlerror());
        write(STDERR_FILENO, message_buffer, msg_len);
        return LIBRARY_ERROR;
    }

    cos_derivative_func_ptr = dlsym(loaded_library, "cos_derivative");
    area_func_ptr = dlsym(loaded_library, "area");

    if (!cos_derivative_func_ptr || !area_func_ptr)
    {
        const char error_msg[] = "Critical error: functions not found\n";
        write(STDERR_FILENO, error_msg, sizeof(error_msg));
        dlclose(loaded_library);
        return LIBRARY_ERROR;
    }

    {
        const char *welcome_text = 
            "Dynamic Loading Application\n"
            "Available commands:\n"
            "0 - toggle between algorithm implementations\n"
            "1 a dx - derivative of cos(x) at point a with step dx\n"
            "2 a b - area with sides a and b\n"
            "exit - exit program\n"
            "Enter command: ";
        write(STDOUT_FILENO, welcome_text, strlen(welcome_text));
    }

    int read_bytes;

    while ((read_bytes = read(STDIN_FILENO, message_buffer, BUFFER_SIZE - 1)) > 0)
    {
        message_buffer[read_bytes] = '\0';

        char *user_command = strtok(message_buffer, " \t\n");
        if (!user_command) continue;

        if (strcmp(user_command, "0") == 0)
        {
            switch_library(library_files, &loaded_library, &active_lib, &cos_derivative_func_ptr, &area_func_ptr);
        }
        else if (strcmp(user_command, "1") == 0)
        {
            execute_derivative(cos_derivative_func_ptr);
        }
        else if (strcmp(user_command, "2") == 0)
        {
            execute_area(area_func_ptr);
        }
        else if (strcmp(user_command, "exit") == 0)
        {
            const char* exit_msg = "Exiting program.\n";
            write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
            break;
        }
        else
        {
            char error_msg[120];
            int len = snprintf(error_msg, 120, "Unknown command '%s'. Use: 0, 1, 2, or exit\n", user_command);
            write(STDOUT_FILENO, error_msg, len);
        }

        write(STDOUT_FILENO, "> ", 2);
    }

    if (loaded_library) dlclose(loaded_library);
    return SUCCESS;
}