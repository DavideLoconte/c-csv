//
// Created by Davide on 29/10/2021.
//

#ifndef C_CSV__UTILS_H
#define C_CSV__UTILS_H

#define STRING_TYPE_INT 1
#define STRING_TYPE_FLOAT 2
#define STRING_TYPE_TEXT 3

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/**
 * Create a copy of the string in the heap
 * @param string the string
 * @param len length of the string
 * @return a copy of the string
 */
char *string_duplicate(const char *string, size_t len);

/**
 * Return the type stored in the string
 * @return TYPE_TEXT, TYPE_FLOAT or TYPE_INT accordingly to the data of the string
 */
char string_type(const char *str);

#endif //C_CSV__UTILS_H
