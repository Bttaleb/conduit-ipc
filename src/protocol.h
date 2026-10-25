#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>

// 1. WIRE FORMAT
// Frame: [len: u32][type: u8][payload: len bytes]
// All integers are little-endian (native); both ends run on the same machine
// Every payload is at least 4 bytes (job_id) so len == 0 is malformed

// 2. CONSTANTS
enum {
    MSG_JOB     = 1,
    MSG_RESULT  = 2
};

#define HEADER_SIZE 5 // len (4) + type (1)
#define MAX_PAYLOAD (64 * 1024) // the reciever's guard
#define RESULT_PAYLOAD_SIZE 20 // job_id (4) + word_count (8) + compute_ns (8)

// 3. IN-MEMORY TYPES
struct result {
    uint32_t job_id;
    uint64_t word_count;
    uint64_t compute_ns;
};

// 4. FRAMING: send/receive one whole message

// send_msg: send one whole framed message to fd
// fd -> where to write
// type -> which message
// payload -> bytes to send
// len -> how many
// Returns:
// 0 on success
// -1 on failure (errno says why)
// PRECONDITION: caller must ignore SIGPIPE
ssize_t send_msg(int fd, uint8_t type, const void *payload, uint32_t len);

// recv_msg: read one whole framed message from fd
// type -> out parameter: filled with message type
// buf -> caller owned destination, cap bytes
// Returns:
// len on success (payload length; *type is set)
// 0 on clean EOF before the header (peer finished)
// -1 on truncated message, read error, or bad len (errno = EMSGSIZE)
// After -1 the stream is out of sync; caller should close it
ssize_t recv_msg(int fd, uint8_t *type, void *buf, size_t cap);

// 5. SERIALIZATION: struct <-> exact wire bytes
// encode_result: write r into out at protocol offsets (0, 4, 12).
//     - Writes exactly RESULT_PAYLOAD_SIZE
void encode_result(const struct result *r, uint8_t *out);

// decode_result(): read the fields back out of the bytes in 'in'.
void decode_result(const uint8_t *in, struct result *r);

#endif
