



#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 50
#define ARGLEN 128
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20
#define MAX_JOBS 50

typedef struct {
    pid_t pid;
    char cmdline[MAX_LEN];
    int active;
} Job;

// Global job list
extern Job jobs[MAX_JOBS];
extern int job_count;

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist, int background, const char* cmdline);
int handle_builtin(char** arglist);
void save_command_history(const char* cmd);
void print_saved_history(void);
char* get_saved_command(int n);

// Jobs management
void add_job(pid_t pid, const char* cmdline);
void remove_job(pid_t pid);
void print_jobs(void);
void reap_terminated_jobs(void);

#endif
