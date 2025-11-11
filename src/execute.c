



#include "shell.h"

Job jobs[MAX_JOBS];
int job_count = 0;

/* -------------------- Utility Helpers -------------------- */
int find_symbol(char** arglist, const char* symbol) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], symbol) == 0)
            return i;
    }
    return -1;
}

void split_args(char** arglist, int index, char** left, char** right) {
    int i;
    for (i = 0; i < index; i++)
        left[i] = arglist[i];
    left[i] = NULL;

    int j = 0;
    for (i = index + 1; arglist[i] != NULL; i++)
        right[j++] = arglist[i];
    right[j] = NULL;
}

/* -------------------- Job Management -------------------- */
void add_job(pid_t pid, const char* cmdline) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].cmdline, cmdline, MAX_LEN);
        jobs[job_count].active = 1;
        job_count++;
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid && jobs[i].active) {
            jobs[i].active = 0;
            printf("[+] Background job finished: PID=%d CMD=%s\n", pid, jobs[i].cmdline);
        }
    }
}

void print_jobs(void) {
    printf("\nActive Background Jobs:\n");
    int any = 0;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active) {
            printf("[%d] PID=%d CMD=%s\n", i + 1, jobs[i].pid, jobs[i].cmdline);
            any = 1;
        }
    }
    if (!any) printf("No background jobs.\n");
}

void reap_terminated_jobs(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        remove_job(pid);
}

/* -------------------- Core Execution -------------------- */
int execute(char* arglist[], int background, const char* cmdline) {
    int input_index = find_symbol(arglist, "<");
    int output_index = find_symbol(arglist, ">");
    int append_index = find_symbol(arglist, ">>");
    int pipe_index = find_symbol(arglist, "|");

    /* ---------- PIPE: cmd1 | cmd2 ---------- */
    if (pipe_index != -1) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe failed");
            return -1;
        }

        char* left[MAXARGS + 1];
        char* right[MAXARGS + 1];
        split_args(arglist, pipe_index, left, right);

        pid_t pid1 = fork();
        if (pid1 == 0) {
            // Left side: output -> pipe
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            execvp(left[0], left);
            perror("pipe: command1 failed");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            // Right side: input <- pipe
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);

            // 🔧 Handle redirection on right-hand command
            int output_index = find_symbol(right, ">");
            int append_index = find_symbol(right, ">>");
            int input_index = find_symbol(right, "<");

            int saved_stdout = -1, saved_stdin = -1;

            // Input redirection (<)
            if (input_index != -1) {
                char* filename = right[input_index + 1];
                if (!filename) {
                    fprintf(stderr, "syntax error: no file after <\n");
                    exit(1);
                }
                int fd_in = open(filename, O_RDONLY);
                if (fd_in == -1) {
                    perror("open");
                    exit(1);
                }
                saved_stdin = dup(STDIN_FILENO);
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
                right[input_index] = NULL;
            }

            // Output redirection (> or >>)
            if (output_index != -1 || append_index != -1) {
                int idx = (output_index != -1) ? output_index : append_index;
                char* filename = right[idx + 1];
                if (!filename) {
                    fprintf(stderr, "syntax error: no file after > or >>\n");
                    exit(1);
                }
                int flags = (output_index != -1)
                            ? (O_WRONLY | O_CREAT | O_TRUNC)
                            : (O_WRONLY | O_CREAT | O_APPEND);
                int fd_out = open(filename, flags, 0644);
                if (fd_out == -1) {
                    perror("open");
                    exit(1);
                }
                saved_stdout = dup(STDOUT_FILENO);
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
                right[idx] = NULL;
            }

            execvp(right[0], right);
            perror("pipe: command2 failed");
            exit(1);
        }

        close(fd[0]);
        close(fd[1]);

        if (background) {
            printf("[+] Background pipe started: PIDs=%d,%d\n", pid1, pid2);
            add_job(pid1, cmdline);
            add_job(pid2, cmdline);
            return 0;
        } else {
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            return 0;
        }
    }

    /* ---------- Output Redirection (> , >>) ---------- */
    int saved_stdout = -1;
    if (output_index != -1 || append_index != -1) {
        int idx = (output_index != -1) ? output_index : append_index;
        char* filename = arglist[idx + 1];
        if (!filename) {
            fprintf(stderr, "syntax error: no file after > or >>\n");
            return -1;
        }

        int flags = (output_index != -1)
                    ? (O_WRONLY | O_CREAT | O_TRUNC)
                    : (O_WRONLY | O_CREAT | O_APPEND);

        int fd = open(filename, flags, 0644);
        if (fd == -1) {
            perror("open");
            return -1;
        }

        saved_stdout = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        arglist[idx] = NULL;
    }

    /* ---------- Input Redirection (<) ---------- */
    int saved_stdin = -1;
    if (input_index != -1) {
        char* filename = arglist[input_index + 1];
        if (!filename) {
            fprintf(stderr, "syntax error: no file after <\n");
            return -1;
        }

        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open");
            return -1;
        }

        saved_stdin = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        close(fd);
        arglist[input_index] = NULL;
    }

    /* ---------- Normal Command (with Background) ---------- */
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        execvp(arglist[0], arglist);
        perror("command not found");
        exit(1);
    } else {
        if (background) {
            printf("[+] Background process started: PID=%d\n", pid);
            add_job(pid, cmdline);
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    /* ---------- Restore file descriptors ---------- */
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }

    return 0;
}
