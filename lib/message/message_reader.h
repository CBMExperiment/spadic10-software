/**
 * \file
 * \author Michael Krieger
 *
 * This module provides another level of abstraction for reading SPADIC
 * messages from input buffers. It saves the user from using the
 * message_read_from_buffer()/::message_is_complete() functions
 * directly and having to manually keeping track of the reading position
 * and state.
 *
 * All functions receiving a pointer `r` to a ::MessageReader object assume
 * that it has been properly allocated and initialized (by
 * message_reader_new()).
 */

#ifndef SPADIC_MESSAGE_READER_H
#define SPADIC_MESSAGE_READER_H

#include <stddef.h>
#include <stdint.h>

typedef struct message_reader MessageReader;
/**<
 * Context for reading SPADIC messages from buffers.
 *
 * Manages input buffers, keeps track of the reading position and saves
 * the state of partially read messages across buffer boundaries.
 */

MessageReader *message_reader_new(void);
/**<
 * Allocate and initialize a new message reader.
 * \return Pointer to created message reader, `NULL` if unsuccessful.
 */
void message_reader_delete(MessageReader *r);
/**<
 * Clean up and deallocate a message reader.
 *
 * References to all buffers that have been added will be lost. If they
 * need to be deallocated, first use message_reader_get_depleted(),
 * possibly also message_reader_reset().
 */
void message_reader_reset(MessageReader *r);
/**<
 * Reset a message reader to its initial state.
 *
 * All buffers that have been added before will be marked as "depleted"
 * and can be returned by message_reader_get_depleted().
 */
int message_reader_add_buffer(MessageReader *r, const uint16_t *buf, size_t len);
/**<
 * Add a new buffer with `len` words to a message reader.
 * \return Zero if successful, non-zero otherwise.
 *
 * The message reader pointed to by `r` will be unmodified in case of
 * failure.
 *
 * The values of `buf` and `len` are not checked in this function.  If
 * `buf` is `NULL`, the behaviour of the reader is undefined. For the
 * reader to do something *useful*, `len` should be positive.
 *
 * More than one buffer can be added to a message reader. They will be
 * consumed in the order in which they were added, keeping incomplete
 * messages across buffers. The buffers are therefore effectively
 * concatenated.
 *
 * Once all words from a buffer have been consumed using
 * message_reader_get_message(), it is marked as "depleted" and can be
 * returned by message_reader_get_depleted().
 */
const uint16_t *message_reader_get_depleted(MessageReader *r);
/**<
 * \return Next depleted buffer, NULL if no depleted buffers are left.
 */
Message *message_reader_get_message(MessageReader *r);
/**<
 * Read the next message.
 *
 * \return Pointer to a message object, if available, `NULL` otherwise.
 *
 * Normally, `NULL` is returned because all buffers are depleted. Check
 * this with message_reader_is_empty().
 *
 * The returned messages are always complete (message_is_complete()).
 */
int message_reader_is_empty(MessageReader *r);
/**<
 * \return Non-zero if all buffers are depleted.
 *
 * Add more buffers to read from with message_reader_add_buffer().
 *
 * If message_reader_get_message() returned `NULL` and the reader is not
 * empty (this function returns zero), an internal error has occurred.
 */

#endif
