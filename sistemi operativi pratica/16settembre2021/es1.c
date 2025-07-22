#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + NAME_MAX + 1))
#define WATCH_DIR "./exec"

void execute_command_from_filename(const char *filename) {
    // Copia il nome del file in una stringa modificabile
    char *cmd = strdup(filename);
    if (!cmd) {
        perror("strdup");
        return;
    }

    // Parsing dei parametri dividendo su spazi
    char *args[64];
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (i == 0) {
        fprintf(stderr, "No command to execute.\n");
        free(cmd);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Figlio: esegue il comando
        execvp(args[0], args);
        perror("execvp"); // Se fallisce
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Padre: aspetta il figlio e poi rimuove il file
        wait(NULL);

        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/%s", WATCH_DIR, filename);
        if (unlink(filepath) != 0) {
            perror("unlink");
        }
    } else {
        perror("fork");
    }

    free(cmd);
}

int main() {
    // Crea la directory exec se non esiste
    mkdir(WATCH_DIR, 0755);

    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(fd, WATCH_DIR, IN_CREATE);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Monitoring directory '%s'...\n", WATCH_DIR);

    char buffer[BUF_LEN];
    while (1) {
        ssize_t length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            perror("read");
            break;
        }

        ssize_t i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (event->len && (event->mask & IN_CREATE)) {
                // Stampa e esegue il comando
                printf("Detected file: '%s'\n", event->name);
                execute_command_from_filename(event->name);
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}
