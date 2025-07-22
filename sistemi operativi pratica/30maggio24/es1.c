#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CMDLINE 4096

void fatal(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int target_pid = atoi(argv[1]);
    if (target_pid <= 0) {
        fprintf(stderr, "PID non valido.\n");
        return EXIT_FAILURE;
    }

    // 1. Ottieni il path dell'eseguibile dal link simbolico /proc/<pid>/exe
    char exe_link[64];
    snprintf(exe_link, sizeof(exe_link), "/proc/%d/exe", target_pid);

    char exe_path[PATH_MAX];
    ssize_t len = readlink(exe_link, exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        fatal("Impossibile leggere il link /proc/<pid>/exe");
    }
    exe_path[len] = '\0'; // readlink non aggiunge '\0'

    // 2. Leggi gli argomenti da /proc/<pid>/cmdline con read()
    char cmdline_path[64];
    snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", target_pid);

    int fd = open(cmdline_path, O_RDONLY);
    if (fd == -1) {
        fatal("Impossibile aprire /proc/<pid>/cmdline");
    }

    char buffer[MAX_CMDLINE];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (n <= 0) {
        fprintf(stderr, "Nessun contenuto in /proc/%d/cmdline\n", target_pid);
        return EXIT_FAILURE;
    }

    buffer[n] = '\0'; // sicurezza

    // Conta gli argomenti
    size_t pos = 0;
    int arg_count = 0;
    while (pos < n) {
        arg_count++;
        pos += strlen(&buffer[pos]) + 1;
    }

    // Costruisci l'array argv[]
    char **new_argv = calloc(arg_count + 1, sizeof(char *));
    if (!new_argv) {
        fatal("calloc");
    }
    pos = 0;
    for (int i = 0; i < arg_count; i++) {
        new_argv[i] = &buffer[pos];
        pos += strlen(&buffer[pos]) + 1;
    }
    new_argv[arg_count] = NULL;

    // 3. Esegui il nuovo processo con gli stessi argomenti
    execv(exe_path, new_argv);

    // Se execv fallisce
    fatal("execv fallita");
}
