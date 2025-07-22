/* Estendere il programma stdin2pipe in modo da consentire più di due righe di input.
Per esempio, dato il file cmds che contiene 4 righe
 ls -l
 awk '{print $1}'
 sort
 uniq
l'esecuzione di:
 stdin2pipe < cmds
sia equivalente al comando:
 ls -l | awk '{print $1}' | sort | uniq'*/


 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

char **parse_line(char *line) {
    int capacity = 4;
    int count = 0;
    char **args = malloc(capacity * sizeof(char *));
    if (!args) {
        perror("malloc");
        exit(1);
    }

    line[strcspn(line, "\n")] = '\0';

    char *token = strtok(line, " ");
    while (token != NULL) {
        if (count >= capacity) {
            capacity *= 2;
            char **new_args = realloc(args, capacity * sizeof(char *));
            if (!new_args) {
                perror("realloc");
                free(args);
                exit(1);
            }
            args = new_args;  // 🔁 qui fai esattamente bene: aggiorni args
        }

        args[count++] = token;
        token = strtok(NULL, " ");
    }

    // NULL finale per execvp
    char **new_args = realloc(args, (count + 1) * sizeof(char *));
    if (!new_args) {
        perror("realloc");
        free(args);
        exit(1);
    }
    args = new_args;
    args[count] = NULL;

    return args;
}


int main() {
    int capacity = 10;
    int num_cmds = 0;

    char **lines = malloc(capacity * sizeof(char *));
    char ***args = malloc(capacity * sizeof(char **));

    char buffer[MAX_LINE];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (num_cmds >= capacity) {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(char *));
            args = realloc(args, capacity * sizeof(char **));
        }
        lines[num_cmds] = strdup(buffer);
        args[num_cmds] = parse_line(lines[num_cmds]);
        num_cmds++;
    }

    int prev_pipe_read = -1;

    for (int i = 0; i < num_cmds; i++) {
        int pipefd[2];
        if (i < num_cmds - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            if (i > 0) {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }

            if (i < num_cmds - 1) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            execvp(args[i][0], args[i]);
            perror("execvp");
            exit(1);
        }

        if (prev_pipe_read != -1)
            close(prev_pipe_read);
        if (i < num_cmds - 1) {
            close(pipefd[1]);
            prev_pipe_read = pipefd[0];
        }
    }

    for (int i = 0; i < num_cmds; i++)
        wait(NULL);

    // cleanup (solo per buona pratica)
    for (int i = 0; i < num_cmds; i++) {
        free(args[i]);
        free(lines[i]);
    }
    free(args);
    free(lines);

    return 0;
}
