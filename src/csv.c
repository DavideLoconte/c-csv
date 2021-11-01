//
// Created by Davide on 29/10/2021.
//

#include <ctype.h>
#include "../include/csv.h"

#define BUFFER_SIZE (1024 * 32)
#define FIELD_BUFFER_SIZE 512
#define RECORD_SIZE 8

#define HEADER_FOUND 0X01
#define ESCAPING 0X02
#define ONE_DQUOTE_FOUND 0X04
#define TWO_DQUOTES_FOUND 0X08
#define ENF_OF_FILE 0x10

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
        Buffer *buffer;
        Record *currentRecord;
        Record *header;
        char flags;

        FILE *currentCsv;
        char nextchr;
        char *tempBuffer;
        size_t bufPos;
} ParsingContext;

// Private prototypes

/**
 * Returns a non 0 value if c is a alphanumeric character or a symbol that
 * adheres to TEXTDATA in RFC 4180
 * @param c the caracter
 * @return !0 if the character is TEXTDATA
 */
int is_normal_character(char c);

/**
 * Process the next character
 * @param reader The CsvReader
 * @param pc the current parsing context
 */
void parse_data(CsvReader *reader, ParsingContext *pc);

/**
 * Process the next character assuming it is escaped
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void parse_escaped_data(CsvReader *reader, ParsingContext *pc);

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
void emit_field(CsvReader *reader, ParsingContext *pc);

/**
 * Start escaping the sequence after the current character
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void start_escaped_sequence(CsvReader *reader, ParsingContext *pc);

/**
 * End the escaped sequence on this character and move to the beginning of the
 * next field
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void end_escaped_sequence(CsvReader *reader, ParsingContext *pc);

/**
 * Add a double quote (") to the current buffer
 * @param reader the CsvReader
 * @param pc the current parsing context
 */
void escape_quotes(CsvReader *reader, ParsingContext *pc);

char get_next_character(ParsingContext *pc);

void get_next_line(ParsingContext *pc);


// Public Implementation

CsvReader *csv_reader_alloc(void (*headerCallback)(void *, Record *),
                            void (*recordCallback)(void *, Record *, Record *),
                            void *context)
{
        CsvReader *result = malloc(sizeof(CsvReader));

        if (result == NULL) return NULL;

        result->context = context;
        result->header = headerCallback;
        result->record = recordCallback;

        return result;
}

inline void csv_reader_free(CsvReader *reader) {free(reader);}

void csv_reader_parse(CsvReader *reader, FILE *csvFile)
{
        ParsingContext pc;

        pc.buffer = buffer_alloc(FIELD_BUFFER_SIZE);
        pc.currentRecord = record_alloc(RECORD_SIZE);
        pc.header = NULL;
        pc.flags = 0x00;
        pc.currentCsv = csvFile;
        pc.nextchr = 0;
        pc.tempBuffer = malloc(sizeof (char ) * (BUFFER_SIZE));

        get_next_line(&pc);

        while (!(pc.flags & ENF_OF_FILE)) {
                pc.nextchr = get_next_character(&pc);
                if (pc.flags & ESCAPING) {
                        parse_escaped_data(reader, &pc);
                } else {
                        parse_data(reader, &pc);
                }
        }

        record_free(pc.currentRecord);
        record_free(pc.header);
        buffer_free(pc.buffer);
        free(pc.tempBuffer);
}

// Private

inline char get_next_character(ParsingContext *pc)
{
        char next = pc->tempBuffer[pc->bufPos++];

        if (pc->bufPos > BUFFER_SIZE)
                get_next_line(pc);

        return next;
}

inline void get_next_line(ParsingContext *pc)
{
        pc->bufPos = 0;
        if (!fgets(pc->tempBuffer, BUFFER_SIZE, pc->currentCsv))
                pc->flags |= ENF_OF_FILE;
}

inline int is_normal_character(char c) {
        return (0x20 == c || c == 0x21 || (0x23 <= c && c <= 0x2b) || (0x2d <= c && c <= 0x7e));
}


void emit_record(CsvReader *reader, ParsingContext *pc)
{
        record_append_str_by_value(pc->currentRecord,
                                   pc->buffer->buffer,
                                   pc->buffer->stringLength);
        buffer_reset(pc->buffer);

        if (pc->flags & HEADER_FOUND) {

                if (reader->record)
                        reader->record(reader->context, pc->header, pc->currentRecord);

                record_reset(pc->currentRecord);
        } else {
                pc->flags |= HEADER_FOUND;
                pc->header = pc->currentRecord;
                pc->currentRecord = record_alloc(pc->header->arraySize);

                if (reader->header) {
                        reader->header(reader->context, pc->header);
                }
        }

        get_next_line(pc);
}

void emit_field(CsvReader *reader, ParsingContext *pc)
{
        record_append_str_by_value(pc->currentRecord,
                                   pc->buffer->buffer,
                                   pc->buffer->stringLength);
        buffer_reset(pc->buffer);
}

void start_escaped_sequence(CsvReader *reader, ParsingContext *pc)
{
        pc->flags |= ESCAPING;
        pc->flags &= ~ONE_DQUOTE_FOUND | ~TWO_DQUOTES_FOUND;
        buffer_reset(pc->buffer);
}

void parse_data(CsvReader *reader, ParsingContext *pc)
{
        if (is_normal_character((char) pc->nextchr))
                append_char(pc->buffer, (char) pc->nextchr);

        else if ((char) pc->nextchr == '"')
                start_escaped_sequence(reader, pc);

        else if ((char) pc->nextchr == ',')
                emit_field(reader, pc);

        else if (pc->nextchr == '\n' || pc->nextchr == EOF)
                emit_record(reader, pc);
}

void parse_escaped_data(CsvReader *reader, ParsingContext *pc)
{
        if (pc->nextchr != '"') {
                if (pc->flags & ONE_DQUOTE_FOUND) {
                        end_escaped_sequence(reader, pc);
                        emit_record(reader, pc);
                }
                else {
                        append_char(pc->buffer, (char) pc->nextchr);
                }
        }
        else {
                if (pc->flags & TWO_DQUOTES_FOUND)
                        escape_quotes(reader, pc);
                else if (pc->flags & ONE_DQUOTE_FOUND)
                        pc->flags |= TWO_DQUOTES_FOUND;
                else
                        pc->flags |= ONE_DQUOTE_FOUND;
        }
}

void end_escaped_sequence(CsvReader *reader, ParsingContext *pc)
{
        while (pc->nextchr != ',' && pc->nextchr != '\n')
                pc->nextchr = fgetc(pc->currentCsv);

        pc->flags &= ~ONE_DQUOTE_FOUND | ~TWO_DQUOTES_FOUND | ~ESCAPING;
}

void escape_quotes(CsvReader *reader, ParsingContext *pc)
{
        pc->flags &= ~ONE_DQUOTE_FOUND | ~TWO_DQUOTES_FOUND;
        append_char(pc->buffer, '"');
}


