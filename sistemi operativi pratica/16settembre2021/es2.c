/*Esercizio 2: Linguaggio C: 10 punti
modificare il programma dell'esercizio 1 per fare in modo che execname2 scriva l'output
dell'esecuzione nel file invece che cancellarlo.
Nell'esempio precedente il programma execname2 non emette alcun output ma il comando
cat 'exec/echo ciao mare'
produce come risultato:
ciao mare*/



void execute_command_from_filename(const char *filename, char *dir) {
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
        // Figlio: esegue il comandoc
        char full_path [PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s",dir, filename);
        int fd_file = open(full_path, O_WRONLY);
        dup2(1, fd_file);
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
                execute_command_from_filename(event->name,WATCH_DIR);
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}
