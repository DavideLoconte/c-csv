//
// Created by Davide on 29/10/2021.
//

#ifndef C_CSV__CSV_H
#define C_CSV__CSV_H

#include <stdio.h>

#include "../src/record.h"
#include "../src/buffer.h"

/**
 * CsvReader data structure
 */
typedef struct CsvReader_s {
        void *context;
        void (*header)(void *, Record *);
        void (*record)(void *, Record *, Record *);
} CsvReader;

/**
 * Allocate a CsvReader Instance
 * @param headerCallback a pointer to a function with signature
 *                       void (void *context, Record *header)
 *                       that will be invoked as soon as the parser reads
 *                       the header. Context points to the context object (see
 *                       below), header is a Record structure which contains
 *                       the header fields as an array of strings. If null,
 *                       no function will be invoked
 * @param recordCallback a pointer to a function with signature
 *                       void (void *context, Record *header, Record *record),
 *                       which will be invoked every time the parser reads
 *                       a record. Context points to the context object (see
 *                       below), header and records contains respectively the
 *                       csv header and the current record. If NULL, no function
 *                       will be invoked
 * @param context Context object: a generic void * pointer that is passed to
 *                the header and record callback function as argument. It can
 *                be used to pass generic data to those function
 * @return A CsvReader Instance
 */
CsvReader *csv_reader_alloc(void (*headerCallback)(void *, Record *),
                            void (*recordCallback)(void *, Record *, Record *),
                            void *context);

/**
 * Reads a csv file
 * @param reader the CsvReader instance
 * @param csvFile a FILE * pointer opened with mode 'r' pointing to the csv file
 */
void csv_reader_parse(CsvReader *reader, FILE *csvFile);

/**
 * Destroy a CsvReader Instance
 * @param reader the instance
 */
void csv_reader_free(CsvReader *reader);

#endif //C_CSV__CSV_H
