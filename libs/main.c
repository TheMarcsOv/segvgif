#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>

#define RD 0
#define WR 1

static void on_segfault() {
    printf("SEGFAULT LETS GOOOO!!!!\n");
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("Usage: %s <program> [args]\n", argv[0]);
        return 1;
    }

    printf("Launching process with args: ");
    for (size_t i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    size_t child_args = argc - 2;

    int errpipe[2];
    if (pipe(errpipe) < 0) {
        fprintf(stderr, "Failed to pipe\n");
        return 1;
    }

    pid_t fork_pid = fork();
    if (fork_pid == 0) {
        // Child side
        close(errpipe[RD]);

        //Try non path first, then 
        if(execv(argv[1], &argv[1]) < 0 && errno != ENOENT) {
            dprintf(errpipe[WR], "execve error: %s\n", strerror(errno));
            close(errpipe[WR]);
            return 69;
        }

        if(execvp(argv[1], &argv[1]) < 0) {
            dprintf(errpipe[WR], "execvpe error: %s\n", strerror(errno));
            close(errpipe[WR]);
            return 69;
        }

        exit(33);        
    } else if (fork_pid < -1) {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
        return 1;
    }
    // Parent side

    close(errpipe[WR]);
    char buf[1024];
    ssize_t n = read(errpipe[RD], buf, sizeof(buf));
    close(errpipe[RD]);

    if (n > 0) {
        fprintf(stderr, "Child error: %.*s", (int)n, buf);
    } else if (n < 0) {
        fprintf(stderr, "read error!\n");
        return 1;
    } else {
        printf("No read xd\n");
    }

    while (1) {
        int wstatus = 0;
        if (waitpid(fork_pid, &wstatus, 0) < 0) {
            fprintf(stderr, "waitpid failure %s\n", strerror(errno));
            return 1;
        }

        if (WIFEXITED(wstatus)) {
            printf("Process exited with code: %d\n", WEXITSTATUS(wstatus));
            break;
        }
        if (WIFSIGNALED(wstatus) && WTERMSIG(wstatus) != SIGPIPE) {
            if (WTERMSIG(wstatus) == SIGSEGV) {
                on_segfault();
            } else {
                printf("Process signal and terminated. Signal: %d\n", WTERMSIG(wstatus));
            }
            break;
        }
    }
        
    return 0;
}
