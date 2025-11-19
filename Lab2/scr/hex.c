#include "hex.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

char* hex_add(const char* hex1, const char* hex2)
{
    size_t length1 = strlen(hex1);
    size_t length2 = strlen(hex2);
    size_t max_length = length1 > length2 ? length1 : length2;
    size_t output_size = max_length + 2;

    char *output_buffer = malloc(output_size);
    if (!output_buffer)
    {
        char error_msg[] = "Memory allocation failed\n";
        write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
        return NULL;
    }

    uint8_t overflow = 0;
    uint64_t position = output_size - 2;
    output_buffer[output_size - 1] = '\0';

    for (size_t index = 0; index < max_length; ++index)
    {
        uint8_t digit1 = 0, digit2 = 0;

        if (index < length1)
        {
            char current_char = hex1[length1 - 1 - index];
            digit1 = (current_char >= '0' && current_char <= '9') ? current_char - '0' : current_char - 'a' + 10;
        }

        if (index < length2)
        {
            char current_char = hex2[length2 - 1 - index];
            digit2 = (current_char >= '0' && current_char <= '9') ? current_char - '0' : current_char - 'a' + 10;
        }

        uint8_t total_digits = digit1 + digit2 + overflow;
        overflow = total_digits / 16;
        total_digits %= 16;

        output_buffer[position--] = total_digits < 10 ? total_digits + '0' : total_digits - 10 + 'a';
    }

    if (overflow)
    {
        output_buffer[position--] = overflow < 10 ? overflow + '0' : overflow - 10 + 'a';
    }

    char *final_output = malloc(strlen(output_buffer + position + 1) + 1);
    if (!final_output)
    {
        char error_msg[] = "Memory allocation failed\n";
        write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
        free(output_buffer);
        return NULL;
    }

    strcpy(final_output, output_buffer + position + 1);
    free(output_buffer);
    return final_output;
}

char* hex_div(const char* hex_value, uint64_t divider)
{
    if (divider == 0)
    {
        return NULL;
    }

    if (strcmp(hex_value, "0") == 0)
    {
        return strdup("0");
    }

    size_t hex_length = strlen(hex_value);
    char *division_result = malloc(hex_length + 1);
    if (!division_result)
    {
        char error_msg[] = "Memory allocation failed\n";
        write(STDOUT_FILENO, error_msg, sizeof(error_msg) - 1);
        return NULL;
    }

    uint64_t remainder_value = 0;
    size_t result_index = 0;
    
    for (size_t pos = 0; pos < hex_length; ++pos)
    {
        char current_char = hex_value[pos];
        uint8_t digit_value = (current_char >= '0' && current_char <= '9') ? current_char - '0' : current_char - 'a' + 10;
        
        remainder_value = remainder_value * 16 + digit_value;
        uint64_t quotient_value = remainder_value / divider;
        remainder_value %= divider;

        if (quotient_value > 0 || result_index > 0)
        {
            division_result[result_index++] = quotient_value < 10 ? quotient_value + '0' : quotient_value - 10 + 'a';
        }
    }

    if (result_index == 0)
    {
        division_result[result_index++] = '0';
    }
    division_result[result_index] = '\0';

    return division_result;
}