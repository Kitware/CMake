#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1928
#  pragma warning(disable : 5105) /* macro expansion warning in windows.h */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGE_LENGTH 1023
#define USAGE "Usage: %s <output_file>\n"

// Extracts the jobserver details from the MAKEFLAGS environment variable.
//
// Returns a pointer to either a string of the form "R,W" where R and W are fds
// or "fifo:PATH".
//
// Returns NULL if MAKEFLAGS is not set or does not contain recognized
// jobserver flags.
char* jobserver_auth(char* message)
{
  const char* jobserver_flags[3] = { "--jobserver-auth=", "--jobserver-fds=",
                                     "-J" };
  char* start = NULL;
  char* end;
  char* result;
  size_t len;
  int i;

  char* makeflags = getenv("MAKEFLAGS");
  if (makeflags == NULL) {
    strncpy(message, "MAKEFLAGS not set", MAX_MESSAGE_LENGTH);
    return NULL;
  }

  fprintf(stdout, "MAKEFLAGS: %s\n", makeflags);

  for (i = 0; i < 3; i++) {
    start = strstr(makeflags, jobserver_flags[i]);
    if (start != NULL) {
      start += strlen(jobserver_flags[i]);
      break;
    }
  }

  if (start == NULL) {
    strncpy(message, "No jobserver flags found", MAX_MESSAGE_LENGTH);
    return NULL;
  }

  // Skip leading white space
  while (*start == ' ' || *start == '\t') {
    start++;
  }

  end = strchr(start, ' ');
  if (end == NULL) {
    end = start + strlen(start);
  }
  len = (size_t)(end - start);
  result = (char*)malloc(len + 1);
  strncpy(result, start, len);
  result[len] = '\0';

  return result;
}

#if defined(_WIN32)
#  include <windows.h>

int windows_semaphore(const char* semaphore, char* message)
{
  // Open the semaphore
  HANDLE hSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semaphore);

  if (hSemaphore == NULL) {
#  if defined(_MSC_VER) && _MSC_VER < 1900
    sprintf(message, "Error opening semaphore: %s (%ld)\n", semaphore,
            GetLastError());
#  else
    snprintf(message, MAX_MESSAGE_LENGTH,
             "Error opening semaphore: %s (%ld)\n", semaphore, GetLastError());
#  endif
    return 1;
  }

  strncpy(message, "Success", MAX_MESSAGE_LENGTH);
  return 0;
}
#else
#  include <errno.h>
#  include <fcntl.h>

int test_fd(int read_fd, int write_fd, char* message)
{
  // Detect if the file descriptors are valid
  int read_good = fcntl(read_fd, F_GETFD) != -1;
  int read_error = errno;

  int write_good = fcntl(write_fd, F_GETFD) != -1;
  int write_error = errno;

  if (!read_good || !write_good) {
    snprintf(message, MAX_MESSAGE_LENGTH,
             "Error opening file descriptors: %d (%s), %d (%s)\n", read_fd,
             strerror(read_error), write_fd, strerror(write_error));
    return 1;
  }

  snprintf(message, MAX_MESSAGE_LENGTH, "Success\n");
  return 0;
}

int posix(const char* jobserver, char* message)
{
  int read_fd;
  int write_fd;
  const char* path;

  // First try to parse as "R,W" file descriptors
  if (sscanf(jobserver, "%d,%d", &read_fd, &write_fd) == 2) {
    return test_fd(read_fd, write_fd, message);
  }

  // Then try to parse as "fifo:PATH"
  if (strncmp(jobserver, "fifo:", 5) == 0) {
    path = jobserver + 5;
    read_fd = open(path, O_RDONLY);
    write_fd = open(path, O_WRONLY);
    return test_fd(read_fd, write_fd, message);
  }

  // We don't understand the format
  snprintf(message, MAX_MESSAGE_LENGTH, "Unrecognized jobserver format: %s\n",
           jobserver);
  return 1;
}
#endif

// Takes 1 argument: an outfile to write results to.
int main(int argc, char** argv)
{
  char message[MAX_MESSAGE_LENGTH + 1];
  char* output_file;
  FILE* fp;
  char* jobserver;
  int result;

  if (argc != 2) {
    fprintf(stderr, USAGE, argv[0]);
    return 2;
  }

  output_file = argv[1];
  fp = fopen(output_file, "w");
  if (fp == NULL) {
    fprintf(stderr, "Error opening output file: %s\n", output_file);
    return 2;
  }

  jobserver = jobserver_auth(message);
  if (jobserver == NULL) {
    fprintf(stderr, "%s\n", message);
    return 1;
  }

#if defined(_WIN32)
  result = windows_semaphore(jobserver, message);
#else
  result = posix(jobserver, message);
#endif
  free(jobserver);
  message[MAX_MESSAGE_LENGTH] = '\0';

  return result;
}
