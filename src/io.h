#ifndef IO_H
#define IO_H

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

// read_exact: read exactly n bytes from fd into buf, even if read() delivers them in pieces.
//   fd -> file descriptor to read from
//   buf -> destination, owned by the caller, must hold at least n bytes
//   n -> how many bytes the caller needs
// Returns:
//   n on success (all bytes are in buf)
//   0 on clean EOF (the other side was closed before any bytes arrived)
//   -1 on error, or EOF partway through (truncated); errno says why
ssize_t read_exact(int fd, void *buf, size_t n);

// write_all: sends exactly n bytes from buf to fd, even though write() may accept them in pieces
//   fd -> file descriptor to write to
//   buf -> the source, which we read from (a const)
//   n -> how many bytes the caller requested
// Returns:
//   n -> on success (all n bytes written)
//   -1 -> on failure; errno says why (EPIPE = reader is gone)
// Precondition: The calling process must ignore SIGPIPE;
//  otherwise a missing reader kills the process instead of returning -1
ssize_t write_all(int fd, const void *buf, size_t n);

#endif
