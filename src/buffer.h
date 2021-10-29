//
// Created by Davide on 29/10/2021.
//

#ifndef C_CSV__BUFFER_H
#define C_CSV__BUFFER_H

/**
 * This is a buffer which is internally used by the csv parser to
 * temporarily store the quoted strings
 */
typedef struct Buffer_s {
        char *buffer;
        size_t stringLength;
        size_t bufferLength;
} Buffer;

/**
 * Create a buffer structure
 * @return the buffer
 */
Buffer *buffer_alloc(size_t len);

/**
 * Destroy a buffer structure
 * @param buffer the buffer
 */
void buffer_free(Buffer *buffer);

/**
 * Reset a buffer without reallocating
 * @param buffer the buffer
 */
void buffer_reset(Buffer *buffer);

/**
 * Append a copy of a string at the end of the buffer
 * @param buffer the buffer
 * @param string the string
 * @return the underlying buffer on success, NULL otherwise
 */
char *append_str(Buffer *buffer, const char *string, size_t len);

/**
 * Append a character at the end of the buffer
 * @param buffer the buffer
 * @param character the character
 * @return the underlying buffer on success, NULL otherwise
 */
char *append_char(Buffer *buf, char c);


#endif //C_CSV__BUFFER_H
