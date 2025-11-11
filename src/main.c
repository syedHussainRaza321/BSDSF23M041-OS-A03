



#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    signal(SIGINT, SIG_IGN);

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if (cmdline[0] == '\0') { free(cmdline); continue; }

        // 1️⃣ Reap finished background jobs (avoid zombies)
        reap_terminated_jobs();

        // 2️⃣ Split command by semicolon (;)
        char* command = strtok(cmdline, ";");
        while (command != NULL) {
            // Trim leading/trailing spaces
            while (*command == ' ') command++;
            size_t len = strlen(command);
            while (len > 0 && (command[len - 1] == ' ' || command[len - 1] == '\n'))
                command[--len] = '\0';

            // Handle !n (history)
            if (command[0] == '!') {
                int n = atoi(command + 1);
                char* prev = get_saved_command(n);
                if (prev) {
                    printf("Re-executing: %s\n", prev);
                    free(cmdline);
                    cmdline = strdup(prev);
                    command = strtok(cmdline, ";");
                    continue;
                } else {
                    fprintf(stderr, "No such command: %d\n", n);
                    command = strtok(NULL, ";");
                    continue;
                }
            }

            save_command_history(command);

            // 3️⃣ Detect background execution (&)
            int background = 0;
            for (int i = strlen(command) - 1; i >= 0; i--) {
                if (command[i] == '&') {
                    background = 1;
                    command[i] = '\0';
                } else if (command[i] != ' ' && command[i] != '\t')
                    break;
            }

            if ((arglist = tokenize(command)) != NULL) {
                if (!handle_builtin(arglist))
                    execute(arglist, background, command);

                for (int i = 0; arglist[i] != NULL; i++)
                    free(arglist[i]);
                free(arglist);
            }

            command = strtok(NULL, ";");
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
