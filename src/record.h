//
// Created by Davide on 29/10/2021.
//

#ifndef C_CSV__RECORD_H
#define C_CSV__RECORD_H

#include <stdlib.h>

/**
 * A data structure yelding the data contained in a record
 * fields is a array of strings, containing the actual value,
 * size is the number of fields
 * buffer_size the size of the fields vector
 * size <= buffer_size, there are always buffer_size - size free spaces at the end fields
 */
typedef struct Record_s {
        char **fields;
        size_t arraySize;
        size_t bufferSize;
} Record;

/**
 * Allocate a record
 * @return the record, or NULL on error
 */
Record *record_alloc();

/**
 * Free an allocated record
 * @param r the record
 */
void record_free(Record *r);

/**
 * Reset a record without reallocating it
 * @param r the record
 */
void record_reset(Record *r);

/**
 * Copy the field string and store it in the record
 * @param r the record
 * @param field the string
 * @return the underlying array, or NULL on error
 */
char **record_append_str_by_value(Record *r, const char *field, size_t len);

/**
 * Add a string to the record by reference. This value has to be stored in the heap
 * since it will be automatically freed when record_free is invoked
 * @param r the record
 * @param field a string stored in the heap
 * @return the underlying array, or NULL on error
 */
char **record_append_str_by_reference(Record *r, char *field);



#endif //C_CSV__RECORD_H
