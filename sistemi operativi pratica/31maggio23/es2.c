#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_ARGS 256
#define MAX_ENVS 256

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PID>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char path[PATH_MAX];
    char cmdline[8192];
    char environ[8192];
    char cwd[PATH_MAX];

    // === Leggi cmdline ===
    snprintf(path, sizeof(path), "/proc/%s/cmdline", argv[1]);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Errore apertura cmdline");
        return EXIT_FAILURE;
    }
    ssize_t len = read(fd, cmdline, sizeof(cmdline) - 1);
    close(fd);
    if (len <= 0) {
        perror("Errore lettura cmdline");
        return EXIT_FAILURE;
    }
    cmdline[len] = '\0';

    // === Leggi cwd ===
    snprintf(path, sizeof(path), "/proc/%s/cwd", argv[1]);
    ssize_t l = readlink(path, cwd, sizeof(cwd) - 1);
    if (l == -1) {
        perror("Errore lettura cwd");
        return EXIT_FAILURE;
    }
    cwd[l] = '\0';

    // === Leggi environ ===
    snprintf(path, sizeof(path), "/proc/%s/environ", argv[1]);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Errore apertura environ");
        return EXIT_FAILURE;
    }
    len = read(fd, environ, sizeof(environ) - 1);
    close(fd);
    if (len <= 0) {
        perror("Errore lettura environ");
        return EXIT_FAILURE;
    }
    environ[len] = '\0';

    // === Costruisci argv[] e envp[] ===
    char *argv_new[MAX_ARGS];
    int argc_new = 0;
    char *p = cmdline;
    while (p < cmdline + len && argc_new < MAX_ARGS - 1) {
        argv_new[argc_new++] = p;
        p += strlen(p) + 1;
    }
    argv_new[argc_new] = NULL;

    char *envp_new[MAX_ENVS];
    int envc_new = 0;
    p = environ;
    while (p < environ + len && envc_new < MAX_ENVS - 1) {
        envp_new[envc_new++] = p;
        p += strlen(p) + 1;
    }
    envp_new[envc_new] = NULL;

    // === Fork ed execve ===
    pid_t pid = fork();
    if (pid == -1) {
        perror("Errore fork");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // Figlio
        if (chdir(cwd) == -1) {
            perror("Errore cambio directory");
            exit(EXIT_FAILURE);
        }
        execve(argv_new[0], argv_new, envp_new);
        perror("execve fallita");
        exit(EXIT_FAILURE);
    } else {
        // Padre
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return EXIT_FAILURE;
        }
    }
}
