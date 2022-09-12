#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>

// This file sets up the environment for each target. You can safely ignore it.

#define STACK_GOAL (0xBFFF0000U-COOKIE)
#define MAX_PAD 0x10000

#define PYTHON3 "/usr/bin/python3"
#define UNBUF "-u"
// the preprocessor macro SOLUTION must be defined, example: -DSOLUTION=sol6.py

void spawn_subprocess(int ret_fd[2]) {
    /*
      Fork/execs a subprocess to execute a python3 program denoted by SOLUTION
      
      Uses two pipes to redirect the subprocess' stdin and stdout and returns,
      in ret_fd, an open read end to the pipe connected to the subprocess'
      stdout, and an open write end to the pipe connected to the subprocess'
      stdin.

      Exits on error
     */

    // file descriptors for two pipes
    enum {READ, WRITE};
    int fd_toparent[2]; // parent <- python subprocess
    int fd_fromparent[2]; // parent -> python subprocess
    int rc; // return code

    // arguments for child process
    char *child_argv[4] = {PYTHON3, UNBUF, SOLUTION, NULL};
    
    rc = pipe(fd_toparent);
    if (rc < 0) {
	fprintf(stderr, "Error creating pipe1\n");
	exit(1);
    }

    rc = pipe(fd_fromparent);
    if (rc < 0) {
	fprintf(stderr, "Error creating pipe2\n");
	exit(1);
    }

    rc = fork();
    if (rc < 0) {
	fprintf(stderr, "Fork failed\n");
        exit(1);
    } 
    else if (rc == 0) { // child process
	close(fd_toparent[READ]); // close read end of toparent
	close(fd_fromparent[WRITE]); // close write end of fromparent
	// redirect stdin and stdout
	if (dup2(fd_fromparent[READ], STDIN_FILENO) != STDIN_FILENO) {
	    fprintf(stderr, "Error redirecting over STDIN\n");
	    exit(1);
	}
	if (dup2(fd_toparent[WRITE], STDOUT_FILENO) != STDOUT_FILENO) {
	    fprintf(stderr, "Error redirecting over STDOUT\n");
	    exit(1);
	}
	// close originals
	close(fd_fromparent[READ]);
	close(fd_toparent[WRITE]);

	execv(child_argv[0], child_argv);
	// shouldn't reach this after execv()
	fprintf(stderr, "Error: failed to execute child program\n");
	exit(1);
    }
    // else the parent process
    close(fd_toparent[WRITE]);
    close(fd_fromparent[READ]);
    ret_fd[READ] = fd_toparent[READ];
    ret_fd[WRITE] = fd_fromparent[WRITE];

    return;
}

int main(int argc, char *argv[], char *envp[])
{
#ifdef COOKIE
	// Advance stack by a different amount for each student, so answers
	// are unique.
	alloca(COOKIE);
#endif
#ifdef MINIASLR
	// Optionally, advance stack pointer by a random amount on each run.
	// Only used for target6.
	unsigned int r;
	getentropy(&r, sizeof(unsigned int));
	alloca(r & 0xFF);
#endif

	// Move argv from top of stack to here, and clear envp, to prevent
	// shortcuts.
	char **_argv = alloca(sizeof(char *)*(argc+1));
	_argv[argc] = NULL;
	for (int i=argc-1; i >= 0; i--) {
	  size_t len = strlen(argv[i])+1;
	  _argv[i] = alloca(len);
	  strncpy(_argv[i], argv[i], len);
	  memset(argv[i], 0, len);
	  argv[i] = NULL;
	}
	argv = NULL;
	char **_envp = alloca(sizeof(char *));
	_envp[0] = NULL;
	for (int i=0; envp[i] != 0; i++) {
	  size_t len = strlen(envp[i])+1;
	  memset(envp[i], 0, len);
	  envp[i] = NULL;
	}
	envp = NULL;

	// Make debugging easier by advancing the stack to a position that's
	// invariant of the size of the arguments and environment.
	char *esp = alloca(0);
	if ((esp < (char *)STACK_GOAL) || (esp - (char *)STACK_GOAL > MAX_PAD)) {
		fprintf(stderr, "Can't normalize stack position %p to %p\n", esp, STACK_GOAL);
		return 1;
	}
	alloca(esp - (char *)STACK_GOAL);
#ifdef MINIASLR
	// 2020/03/19: Fix for target6
	alloca(r & 0xFF);
#endif

	// Make it harder to overwrite main's stack frame.
	volatile int canary = 0xB000DEAD;

	int ret = _main(argc, _argv, _envp);

	if (canary != 0xB000DEAD) {
		fprintf(stderr, "Uh oh, the canary is dead.\n" \
				"Don't overwrite the stack frame for main().\n");
	}
	return ret;
}
