#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

#define BUFFER_SIZE 1024

void process_derivative()
{
    char* a_str = strtok(NULL, " \t\n");
    char* dx_str = strtok(NULL, " \t\n");
    
    char output[BUFFER_SIZE];

    if (a_str && dx_str)
    {
        float a_val = atof(a_str);
        float dx_val = atof(dx_str);
        float result = cos_derivative(a_val, dx_val);
        
        int len = snprintf(output, BUFFER_SIZE, "Derivative of cos at %.2f: %.6f\n", a_val, result);
        write(STDOUT_FILENO, output, len);
    }
    else
    {
        const char* error_msg = "Error: two arguments required - a and dx\n";
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
    }
}

void process_area()
{
    char* a_str = strtok(NULL, " \t\n");
    char* b_str = strtok(NULL, " \t\n");
    
    char output[BUFFER_SIZE];

    if (a_str && b_str)
    {
        float a_val = atof(a_str);
        float b_val = atof(b_str);
        float result = area(a_val, b_val);
        
        int len = snprintf(output, BUFFER_SIZE, "Area with sides %.2f and %.2f: %.6f\n", a_val, b_val, result);
        write(STDOUT_FILENO, output, len);
    }
    else
    {
        const char* error_msg = "Error: two arguments required - a and b\n";
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
    }
}

int main()
{
    {
        const char* welcome_msg = 
            "Static Linking Application\n"
            "Available commands:\n"
            "1 a dx - derivative of cos(x) at point a with step dx\n"
            "2 a b - area of figure with sides a and b\n"
            "exit - terminate program\n"
            "Enter command: ";
        write(STDOUT_FILENO, welcome_msg, strlen(welcome_msg));
    }

    int bytes_read = 0;
    char input_line[BUFFER_SIZE];

    while((bytes_read = read(STDIN_FILENO, input_line, BUFFER_SIZE - 1)) > 0)
    {
        input_line[bytes_read] = 0;

        char* command = strtok(input_line, " \t\n");
        if (!command) continue;

        if (strcmp(command, "1") == 0)
        {
            process_derivative();
        }
        else if (strcmp(command, "2") == 0)
        {
            process_area();
        }
        else if (strcmp(command, "exit") == 0)
        {
            const char* bye_msg = "Program terminated.\n";
            write(STDOUT_FILENO, bye_msg, strlen(bye_msg));
            break;
        }
        else
        {
            char error_msg[100];
            int len = snprintf(error_msg, 100, "Unknown command: '%s'. Use '1', '2' or 'exit'\n", command);
            write(STDOUT_FILENO, error_msg, len);
        }

        write(STDOUT_FILENO, "> ", 2);
    }

    return 0;
}