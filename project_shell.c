#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 100
#define MAX_HISTORY 100

char* history[MAX_HISTORY];
int history_count = 0;
pid_t child_pid = -1;

void handle_sigint(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGINT);
    } else {
        printf("\nsh> ");
        fflush(stdout);
    }
}

void add_to_history(char* cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    }
}

char** parse_args(char* cmd) {
    static char* args[MAX_ARGS];
    int i = 0;
    args[i] = strtok(cmd, " \t\n");
    while (args[i] && i < MAX_ARGS - 1) {
        args[++i] = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

int execute_command(char* cmd) {
    int in_fd = -1, out_fd = -1, append = 0;
    char* in_file = NULL;
    char* out_file = NULL;

    char* token = strtok(cmd, " \n");
    char* args[MAX_ARGS];
    int argc = 0;

    while (token) {
        if (strcmp(token, "<") == 0) {
            in_file = strtok(NULL, " \n");
        } else if (strcmp(token, ">>") == 0) {
            out_file = strtok(NULL, " \n");
            append = 1;
        } else if (strcmp(token, ">") == 0) {
            out_file = strtok(NULL, " \n");
            append = 0;
        } else {
            args[argc++] = token;
        }
        token = strtok(NULL, " \n");
    }
    args[argc] = NULL;

    if (args[0] == NULL) return 0;

    // Check for built-in 'cd'
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) args[1] = getenv("HOME");
        if (chdir(args[1]) != 0) {
            perror("cd failed");
            return 1;
        }
        return 0;
    }

    // Check for built-in 'exit'
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    if ((child_pid = fork()) == 0) {
        if (in_file) {
            in_fd = open(in_file, O_RDONLY);
            if (in_fd < 0) {
                perror("Input file open failed");
                exit(1);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_file) {
            out_fd = open(out_file, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
            if (out_fd < 0) {
                perror("Output file open failed");
                exit(1);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else {
        int status;
        waitpid(child_pid, &status, 0);
        child_pid = -1;
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
}

void handle_pipe(char* line) {
    char* cmds[10];
    int n = 0;
    cmds[n] = strtok(line, "|");
    while (cmds[n] && n < 9) {
        cmds[++n] = strtok(NULL, "|");
    }

    int i, fd[2], in_fd = 0;

    for (i = 0; i < n - 1; i++) {
        pipe(fd);
        if ((child_pid = fork()) == 0) {
            dup2(in_fd, 0);
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            execute_command(strdup(cmds[i]));
            exit(0);
        } else {
            wait(NULL);
            close(fd[1]);
            in_fd = fd[0];
        }
    }

    if ((child_pid = fork()) == 0) {
        dup2(in_fd, 0);
        execute_command(strdup(cmds[i]));
        exit(0);
    } else {
        wait(NULL);
    }

    child_pid = -1;
}

void run_line(char* line) {
    char* command;
    char* saveptr1;

    command = strtok_r(line, ";", &saveptr1);

    while (command) {
        while (*command == ' ') command++;

        char* subcmd;
        char* saveptr2;
        int run_next = 1;

        subcmd = strtok_r(command, "&&", &saveptr2);
        while (subcmd) {
            while (*subcmd == ' ') subcmd++;

            if (run_next && *subcmd != '\0') {
                if (strchr(subcmd, '|')) {
                    handle_pipe(subcmd);
                    run_next = 1;
                } else {
                    int status = execute_command(subcmd);
                    run_next = (status == 0);
                }
            } else {
                run_next = 0;
            }

            subcmd = strtok_r(NULL, "&&", &saveptr2);
        }

        command = strtok_r(NULL, ";", &saveptr1);
    }
}

int main() {
    char line[MAX_CMD_LEN];

    signal(SIGINT, handle_sigint);

    while (1) {
        printf("sh> ");
        fflush(stdout);

        if (!fgets(line, MAX_CMD_LEN, stdin)) break;

        if (strcmp(line, "\n") == 0) continue;

        add_to_history(line);

        if (strncmp(line, "history", 7) == 0) {
            for (int i = 0; i < history_count; i++) {
                printf("%d: %s", i + 1, history[i]);
            }
            continue;
        }

        run_line(line);
    }

    return 0;
}
