//
// Created by Davide on 29/10/2021.
//


#include "utils.h"

char *string_duplicate(const char *string, size_t len)
{
        char *duplicate = malloc(sizeof(char) * (len + 1));
        strcpy(duplicate, string);
        return duplicate;
}


char string_type(const char *str)
{
        char type = STRING_TYPE_INT;
        size_t i = 0;
        char accept_sign = 1;
        char decimal_found = 0;

        // Left trim the string
        while (isspace(str[i]))
                i++;

        while (str[i] != 0) {

                if (accept_sign && (str[i] == '+' || str[i] == '-')) {
                        accept_sign = 0;
                }
                        // Search for the first decimal
                else if (type <= STRING_TYPE_FLOAT && str[i] == '.' && !decimal_found) {
                        decimal_found = 1;
                        type = STRING_TYPE_FLOAT;
                } else if (!isdigit(str[i])) {

                        // Right trimming
                        // the first non digit is a space, so find the first character
                        // which is not a space
                        while (isspace(str[i]))
                                i++;

                        // It s the end of the string, so break
                        if (str[i] == 0) {
                                break;
                        } else { // It is a character and the string is text
                                return STRING_TYPE_TEXT;
                        }
                }
                i++;
        }
        return type;
}