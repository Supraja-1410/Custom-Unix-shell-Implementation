#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LEN 1024
#define MAX_ARGS 100

/* ------------------- Global Variables ------------------- */
char *line_buf = NULL;      // input buffer
size_t line_cap = 0;        // buffer capacity
char *args[MAX_ARGS];       // tokenized arguments

/* ------------------- Utility Functions ------------------- */

/* remove leading and trailing spaces */
char *trim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }
    return s;
}

/* break input string into arguments */
void parseInput(char *input) {
    int i = 0;
    char *tok = strtok(input, " \t\n");
    while (tok != NULL && i < MAX_ARGS - 1) {
        args[i++] = tok;
        tok = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

/* simple command executor */
void executeCommand() {
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            printf("Shell: Incorrect command\n");
        }
        exit(0);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
    }
}

/* ------------------- Signal Handler ------------------- */
void sigHandler(int sig) {
    if (sig == SIGINT || sig == SIGTSTP) {
        printf("\n"); // avoid terminating shell
    }
}

/* ------------------- Redirection ------------------- */
void executeCommandRedirection(char *cmd, char *file) {
    parseInput(cmd);
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);

        if (execvp(args[0], args) == -1) {
            printf("Shell: Incorrect command\n");
        }
        exit(0);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
    }
}

/* ------------------- Sequential Execution ------------------- */
void executeSequentialCommands(char *line) {
    char *cursor = line;
    char *cmd;

    while ((cmd = strsep(&cursor, "##")) != NULL) {
        cmd = trim(cmd);
        if (strlen(cmd) == 0) continue;

        parseInput(cmd);
        if (args[0] == NULL) continue;

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                char *home = getenv("HOME");
                if (home) chdir(home);
            } else {
                if (chdir(args[1]) != 0)
                    printf("Shell: Incorrect command\n");
            }
        } else if (strcmp(args[0], "exit") == 0) {
            printf("Exiting shell...\n");
            exit(0);
        } else {
            executeCommand();
        }
    }
}

/* ------------------- Parallel Execution ------------------- */
void executeParallelCommands(char *line) {
    char *cursor = line;
    char *cmd;
    pid_t pid;
    int child_count = 0;

    while ((cmd = strsep(&cursor, "&&")) != NULL) {
        cmd = trim(cmd);
        if (strlen(cmd) == 0) continue;

        parseInput(cmd);
        if (args[0] == NULL) continue;

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                char *home = getenv("HOME");
                if (home) chdir(home);
            } else {
                if (chdir(args[1]) != 0)
                    printf("Shell: Incorrect command\n");
            }
        } else if (strcmp(args[0], "exit") == 0) {
            printf("Exiting shell...\n");
            exit(0);
        } else {
            pid = fork();
            if (pid == 0) {
                if (execvp(args[0], args) == -1) {
                    printf("Shell: Incorrect command\n");
                }
                exit(0);
            } else if (pid > 0) {
                child_count++;
            } else {
                perror("fork");
            }
        }
    }

    for (int i = 0; i < child_count; i++) {
        wait(NULL);
    }
}

/* ------------------- Pipelines ------------------- */
void executePipelines(char *line) {
    char *cmds[50];
    int count = 0;

    cmds[count++] = trim(strtok(line, "|"));
    char *tok;
    while ((tok = strtok(NULL, "|")) != NULL) {
        cmds[count++] = trim(tok);
    }

    int fd[2], in_fd = 0;

    for (int i = 0; i < count; i++) {
        pipe(fd);
        pid_t pid = fork();

        if (pid == 0) {
            dup2(in_fd, 0);
            if (i < count - 1) dup2(fd[1], 1);
            close(fd[0]);

            parseInput(cmds[i]);
            if (execvp(args[0], args) == -1) {
                printf("Shell: Incorrect command\n");
            }
            exit(0);
        } else if (pid > 0) {
            wait(NULL);
            close(fd[1]);
            in_fd = fd[0];
        } else {
            perror("fork");
        }
    }
}

/* ------------------- MAIN ------------------- */
int main() {
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);

    while (1) {
        char cwd[MAX_LEN];
        if (!getcwd(cwd, sizeof(cwd))) strcpy(cwd, "");
        printf("%s$", cwd);

        ssize_t nread = getline(&line_buf, &line_cap, stdin);
        if (nread == -1) break;

        char *input = trim(line_buf);
        if (*input == '\0') continue;

        /* Command type detection */
        if (strchr(input, '>')) {
            char *arrow = strchr(input, '>');
            *arrow = '\0';
            char *cmd = trim(input);
            char *outfile = trim(arrow + 1);

            if (*cmd == '\0' || *outfile == '\0') {
                printf("Shell: Incorrect command\n");
            } else {
                executeCommandRedirection(cmd, outfile);
            }
        } else if (strstr(input, "##")) {
            executeSequentialCommands(input);
        } else if (strstr(input, "&&")) {
            executeParallelCommands(input);
        } else if (strchr(input, '|')) {
            executePipelines(input);
        } else {
            parseInput(input);
            if (args[0] == NULL) continue;

            if (strcmp(args[0], "cd") == 0) {
                if (args[1]) {
                    if (chdir(args[1]) != 0)
                        printf("Shell: Incorrect command\n");
                } else {
                    char *home = getenv("HOME");
                    if (home) chdir(home);
                }
            } else if (strcmp(args[0], "exit") == 0) {
                printf("Exiting shell...\n");
                break;
            } else {
                executeCommand();
            }
        }
    }

    free(line_buf);
    return 0;
}
