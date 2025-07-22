#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define MAX_BUF 8192

void fatal(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Legge file proc virtuale (cmdline, environ) con read()
char *read_proc_file(const char *path, ssize_t *length) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) fatal(path);

    char *buf = malloc(MAX_BUF);
    if (!buf) fatal("malloc");

    ssize_t n = read(fd, buf, MAX_BUF - 1);
    if (n == -1) fatal("read");
    close(fd);

    buf[n] = '\0';
    if (length) *length = n;
    return buf;
}

// Costruisce argv o envp a partire da buffer con stringhe \0 separate
char **split_null_strings(char *buffer, ssize_t length, int *count) {
    int cnt = 0;
    ssize_t pos = 0;
    while (pos < length) {
        cnt++;
        pos += strlen(&buffer[pos]) + 1;
    }

    char **arr = calloc(cnt + 1, sizeof(char *));
    if (!arr) fatal("calloc");

    pos = 0;
    for (int i = 0; i < cnt; i++) {
        arr[i] = &buffer[pos];
        pos += strlen(&buffer[pos]) + 1;
    }
    arr[cnt] = NULL;
    if (count) *count = cnt;
    return arr;
}

char *read_exe_path(int pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);

    char exe_path[PATH_MAX];
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len == -1) fatal("readlink exe");

    exe_path[len] = '\0';
    return strdup(exe_path);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int pid = atoi(argv[1]);
    if (pid <= 0) {
        fprintf(stderr, "PID non valido\n");
        return EXIT_FAILURE;
    }

    char *exe_path = read_exe_path(pid);

    char cmdline_path[64];
    snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);
    ssize_t cmd_len;
    char *cmd_buffer = read_proc_file(cmdline_path, &cmd_len);
    char **argv_clone = split_null_strings(cmd_buffer, cmd_len, NULL);

    char environ_path[64];
    snprintf(environ_path, sizeof(environ_path), "/proc/%d/environ", pid);
    ssize_t env_len;
    char *env_buffer = read_proc_file(environ_path, &env_len);
    char **env_clone = split_null_strings(env_buffer, env_len, NULL);

    execve(exe_path, argv_clone, env_clone);
    perror("execve fallita");
    return EXIT_FAILURE;
}
