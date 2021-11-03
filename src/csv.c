//
// Created by Davide on 29/10/2021.
//

#include <ctype.h>
#include "../include/csv.h"

#define BUFFER_SIZE 512
#define RECORD_SIZE 16

#define HEADER_FOUND 0X01
#define PROCESSED_ALL_RECORDS 0x02
#define ESCAPING 0x04
#define DQUOTE_FOUND 0x08

#define DQUOTE 34
#define NEW_LINE 10
#define CARRIAGE_RETURN 13
#define COMMA 44

#define is_line_ending(c) ((c) == NEW_LINE || (c) == EOF)
#define is_separator(c) ((c) == COMMA)
#define is_dquote(c) ((c) == DQUOTE)

/**
 * This structure encapsulate some data
 * structures used throughout the parsing process
 *
 * @param buffer a dynamic string which temporarily stores the field the parser
 *               is currently reading

 * @param currentRecord A dynamic array of strings.
 *                      Once a CSV record has been completely read by the
 *                      parser, this array contains the fields of that record.
 *                      This pointer is then passed as an argument to the
 *                      recordCallback function to process its data.
 *
 * @param header A dynamic array of strings which stores the fields
 *               of the CSV header
 *
 * @param flags The first five bits holds some information on the parser state.
 *               - HEADER_FOUND: The first record (header) has been processed.
 *               - ESCAPING: The character that the parser is reading is
 *                           enclosed in double quotes and must be escaped
 *                           accordingly to the CSV syntax;
 *               - ONE_DQUOTE_FOUND: The parser encountered one double quote
 *                                   in an escaped sequence;
 *               - TWO_DQUOTES_FOUND: Two consecutive double quotes are found
 *                                    in an escaped sequence. When this happens,
 *                                    the final parsed field will contain a
 *                                    single double quote character.
 *                - END_OF_FILE: 1 if EOF is encountered
 *              Remaining bits are currently unused.
 *
 * @param currentCsv FILE * pointer of the CSV file to parse
 * @param nextchr The current character
 * @param tempBuffer Contains the content of the file
 * @param bufPos store the current position in the buffer
 */
typedef struct {
        FILE *currentCsv;
        Buffer *buffer;
        Record *record;
        Record *header;

        int bufferPosition;
        int lastSeparator;

        Buffer *secondaryBuffer;
        int flags;
} ParsingContext;


// Private prototypes

/**
 * Execute the callback function for the currently stored record,
 * then resets the record buffer
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void emit_record(CsvReader *reader, ParsingContext *pc);

/**
 * Takes the buffer stored in the parsing context and stores it
 * as a field in the record data structure, then resets the buffer
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void emit_field(ParsingContext *pc);

/**
 * Move to the next character
 * @param pc
 */
void get_next_character(ParsingContext *pc);
/**
 * Move to the next line
 * @param pc parsing context
 */
void get_next_line(ParsingContext *pc);


void start_escaped_sequence(CsvReader *reader, ParsingContext *pc);
void end_escaped_sequence(CsvReader *reader, ParsingContext *pc);
void get_escaped_sequence(CsvReader *reader, ParsingContext *pc);
char current_char(ParsingContext *pc);
char lookahead(ParsingContext *pc);


// Public Implementation

CsvReader *csv_reader_alloc(void (*headerCallback)(void *, Record *),
                            void (*recordCallback)(void *, Record *, Record *),
                            void *context)
{
        CsvReader *result = malloc(sizeof(CsvReader));
        if (result == NULL)
                return NULL;
        result->context = context;
        result->header = headerCallback;
        result->record = recordCallback;
        return result;
}

inline void csv_reader_free(CsvReader *reader) {
        free(reader);
}

void csv_reader_parse(CsvReader *reader, FILE *csvFile)
{
        ParsingContext pc;

        pc.currentCsv = csvFile;

        pc.buffer = buffer_alloc(BUFFER_SIZE);
        pc.record = record_alloc(RECORD_SIZE);
        pc.bufferPosition = 0;
        pc.secondaryBuffer = NULL;
        pc.header = NULL;
        pc.flags = 0x00;

        get_next_line(&pc);

        do {
                if (is_separator(pc.buffer->buffer[pc.bufferPosition])) {
                        emit_field(&pc);
                        pc.lastSeparator = pc.bufferPosition + 1;
                } else if (is_line_ending(pc.buffer->buffer[pc.bufferPosition])) {
                        emit_field(&pc);
                        emit_record(reader, &pc);
                } else if (is_dquote(pc.buffer->buffer[pc.bufferPosition])) {
                        start_escaped_sequence(reader, &pc);
                        continue;
                }

                get_next_character(&pc);
        } while (!(pc.flags & PROCESSED_ALL_RECORDS));

        buffer_free(pc.buffer);
        record_free(pc.record);

        if (pc.flags & HEADER_FOUND) {
                record_free(pc.header);
        }

        if (pc.secondaryBuffer != NULL) {
                buffer_free(pc.secondaryBuffer);
        }
}

void get_next_character(ParsingContext *pc)
{
        pc->bufferPosition+=1;

        if (pc->bufferPosition >= pc->buffer->bufferLength) {
                pc->buffer->bufferLength *= 2;
                pc->buffer->buffer = realloc(pc->buffer->buffer, sizeof(char) * pc->buffer->bufferLength);

                if (!pc->buffer->buffer) {
                        perror("Cannot alloc memory buffer for csv parsing");
                        abort();
                }

                if(!fgets(pc->buffer->buffer + pc->bufferPosition - 1,
                          pc->buffer->bufferLength - (pc->bufferPosition - 1),
                          pc->currentCsv)) {
                        fprintf(stderr, "Error reading additional data from CSV\n");
                }
        }
}

inline void get_next_line(ParsingContext *pc)
{
        pc->bufferPosition = 0;
        pc->lastSeparator = 0;

        if (!fgets(pc->buffer->buffer, pc->buffer->bufferLength, pc->currentCsv)) {
                if (feof(pc->currentCsv)) {
                        pc->flags |= PROCESSED_ALL_RECORDS;
                } else {
                        perror("Error while reading input CSV");
                        abort();
                }
        }
}


void emit_record(CsvReader *reader, ParsingContext *pc)
{
        if (pc->flags & HEADER_FOUND) {
                if (reader->record != NULL)
                        reader->record(reader->context, pc->header, pc->record);
                record_reset(pc->record);
        } else {
                if (reader->header != NULL)
                        reader->header(reader->context, pc->record);
                pc->flags |= HEADER_FOUND;
                pc->header = pc->record;
                pc->record = record_alloc(pc->header->arraySize);
        }

        get_next_line(pc);
}

void emit_field(ParsingContext *pc)
{
        char *string = pc->buffer->buffer + pc->lastSeparator;
        char stringTermination = string[pc->bufferPosition - pc->lastSeparator];
        string[pc->bufferPosition - pc->lastSeparator] = 0;
        record_append_str(pc->record, string, pc->bufferPosition - pc->lastSeparator);
        string[pc->bufferPosition - pc->lastSeparator] = stringTermination;
}

void start_escaped_sequence(CsvReader *reader, ParsingContext *pc)
{
        if (pc->secondaryBuffer == NULL) {
                pc->secondaryBuffer = buffer_alloc(BUFFER_SIZE);
        }
        pc->flags |= ESCAPING;
        get_next_character(pc);
        get_escaped_sequence(reader, pc);
}

void get_escaped_sequence(CsvReader *reader, ParsingContext *pc)
{
        while(pc->flags & ESCAPING) {
                if (current_char(pc) == 0) {
                        get_next_line(pc);
                }

                if (!is_dquote(current_char(pc))) {
                        buffer_append(pc->secondaryBuffer, pc->buffer->buffer[pc->bufferPosition]);
                } else if (is_dquote(current_char(pc)) && !is_dquote(lookahead((pc)))) {
                        end_escaped_sequence(reader, pc);
                        break;
                } else if (is_dquote(current_char(pc)) && is_dquote(lookahead((pc)))) {
                        buffer_append(pc->secondaryBuffer, pc->buffer->buffer[pc->bufferPosition]);
                        get_next_character(pc);
                }

                get_next_character(pc);
        }
}

void end_escaped_sequence(CsvReader *reader, ParsingContext *pc)
{
        record_append_str(pc->record, pc->secondaryBuffer->buffer, pc->secondaryBuffer->stringLength);
        buffer_reset(pc->secondaryBuffer);
        pc->flags &= ~ESCAPING;

        do {
                if (is_separator(current_char(pc))) {
                        pc->lastSeparator = pc->bufferPosition + 1;
                        get_next_character(pc);
                } else if (is_line_ending(pc->buffer->buffer[pc->bufferPosition])) {
                        emit_record(reader, pc);
                } else {
                        get_next_character(pc);
                        continue;
                }
                break;
        } while (1);
}

inline char current_char(ParsingContext *pc)
{
        return pc->buffer->buffer[pc->bufferPosition];
}

inline char lookahead(ParsingContext *pc)
{
        return pc->buffer->buffer[pc->bufferPosition + 1];
}