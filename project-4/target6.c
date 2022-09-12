#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/random.h>

/*
  Usage: execute this target on its own with no arguments or input. It will
  execute an existing "sol6.py" python program and communicate with it through
  the python program's stdin and stdout.
*/

// only read or write up to BUFF_SIZE
#define BUFF_SIZE 256

int read_len(int fd_read) {
    /*
      Read 4 bytes from the pipe and store the result in an integer. Implements
      a check to avoid returning lengths greater than the allowed BUFF_SIZE, and
      in such cases caps the length returned at BUFF_SIZE.
    */

    int length;
    if (read(fd_read, &length, 4) < 0) {
	fprintf(stderr, "Error reading length");
	exit(1);
    }

    // Don't allow more than the buffer size
    if (length > BUFF_SIZE) {
	return BUFF_SIZE;
    }
    
    // had some weird stuff go on with the POSIX max size of
    // read/write operations so do an extra check, just in case.
    length &= INT_MAX;
    if (length > SSIZE_MAX) {
	return BUFF_SIZE;
    }
    
    return length;
}

void vulnerable(int fd_read, int fd_write) {
    /*
      1) receive 4 bytes from the child process and store them in length
      
      2) write length bytes from buf, containing lorem ispum text, to the child
      process

      3) again read 4 bytes from the child process and store them in length

      4) read length bytes from the child process into buf

      If the first four bytes received in step (4) match a secret random cookie,
      print a special message. Otherwise, write the received bytes to stdout.
    */
    
    int length;
    char buf[BUFF_SIZE + 4]; // put a secret cookie at the end of the buffer
    char *secret_cookie = &(buf[BUFF_SIZE]);
      
    // copy 255 bytes plus a null terminator into buf
    // to be sent to the child process.
    strcpy(buf,
           "Lorem ipsum dolor sit amet, cons"
           "ectetur adipiscing elit. Quisque"
           " consectetur, metus ac efficitur"
           " iaculis, mauris metis pulvinar "
           "magna, non luctus mauris risus e"
           "t nunc. Sed quis convallis leo, "
           "sit amet aliquam urna. Maecenas "
           "iaculis lobortis ultricies amet");

    // read BUFF_SIZE random bytes into secret_cookie
    if (getrandom(secret_cookie, 4, 0) != 4) {
        fprintf(stderr, "Error reading from /dev/urandom");
        return;
    }

    // read the amount of bytes the child process has requested from the buffer
    length = read_len(fd_read);

    // write that amount of bytes to the child process from the buffer
    write(fd_write, buf, length);

    // read the amount of bytes the child process is about to send
    length = read_len(fd_read);

    // read that amount of bytes from the child process into the buffer
    read(fd_read, buf, length);

    // did the child process guess the secret cookie?
    if (memcmp(buf, secret_cookie, 4) == 0) {
        printf("Correct: %lu == %lu\n", *(uint *) secret_cookie, *(uint *) buf);
    }
    else {
        printf("Received:\n");
        write(STDOUT_FILENO, buf, length);
    }
    return;
}

int _main() {
    
    enum {READ, WRITE};
    int fd[2];

    // spawn a subprocess that will execute the python program "sol6.py"
    // and set up pipes for communication
    
    spawn_subprocess(fd); // defined in helper.c

    // from here on we are the parent process and
    //    can receive data from the python program's stdout with the pipe
    //    referenced by fd[READ]
    //    can send data to the python program's stdin with the pipe referenced
    //    by fd[WRITE]
    
    vulnerable(fd[READ], fd[WRITE]);

    // close pipes and exit
    close(fd[READ]);
    close(fd[WRITE]);
    return 0;
    
}
