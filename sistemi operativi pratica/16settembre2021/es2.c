/*Esercizio 2: Linguaggio C: 10 punti
modificare il programma dell'esercizio 1 per fare in modo che execname2 scriva l'output
dell'esecuzione nel file invece che cancellarlo.
Nell'esempio precedente il programma execname2 non emette alcun output ma il comando
cat 'exec/echo ciao mare'
produce come risultato:
ciao mare*/



void execute_command_from_filename(const char *filename, char *dir) {
    char *cmd = strdup(filename);
    if (!cmd) {
        perror("strdup");
        return;
    }

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
        // Figlio: prepara il file per output
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, filename);

        int fd_file = open(full_path, O_WRONLY | O_TRUNC);
        if (fd_file == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        dup2(fd_file, STDOUT_FILENO);
        dup2(fd_file, STDERR_FILENO); // opzionale
        close(fd_file);

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        wait(NULL); // aspetta il figlio
    } else {
        perror("fork");
    }

    free(cmd);
}
