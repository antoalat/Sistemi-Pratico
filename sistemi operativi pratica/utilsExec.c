#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

// Funzione per eseguire un comando di sistema
int execute_command(const char *command) {
    int status = system(command);
    if (status == -1) {
        perror("Errore nell'esecuzione del comando");
    }
    return status;
}

// Funzione per eseguire un comando e catturare l'output
char* execute_command_capture_output(const char *command) {
    FILE *fp;
    char *output = NULL;
    size_t size = 0;

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Errore nell'esecuzione del comando");
        return NULL;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        char *new_output = realloc(output, size + len + 1);
        if (new_output == NULL) {
            perror("Errore nell'allocazione della memoria");
            free(output);
            pclose(fp);
            return NULL;
        }
        output = new_output;
        strcpy(output + size, buffer);
        size += len;
    }

    pclose(fp);
    return output;
}

// Funzione per lanciare un processo figlio
pid_t launch_process(const char *command) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Errore nella creazione del processo figlio");
        return -1;
    } else if (pid == 0) {
        // Siamo nel processo figlio
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        perror("Errore nell'esecuzione del comando");
        exit(EXIT_FAILURE);
    }
    return pid;
}

// Funzione per aspettare la terminazione di un processo figlio
int wait_for_process(pid_t pid) {
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Errore nell'attesa del processo figlio");
        return -1;
    }
    return status;
}

// Funzione per eseguire un comando con argomenti
int execute_command_with_args(const char *command, char *const argv[]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Errore nella creazione del processo figlio");
        return -1;
    } else if (pid == 0) {
        execvp(command, argv);
        perror("Errore nell'esecuzione del comando");
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Errore nell'attesa del processo figlio");
        return -1;
    }
    return status;
}

// Funzione per eseguire un comando con redirezione dell'output
int execute_command_with_output_redirection(const char *command, const char *output_file) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Errore nella creazione del processo figlio");
        return -1;
    } else if (pid == 0) {
        FILE *file = freopen(output_file, "w", stdout);
        if (file == NULL) {
            perror("Errore nell'apertura del file di output");
            exit(EXIT_FAILURE);
        }
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        perror("Errore nell'esecuzione del comando");
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Errore nell'attesa del processo figlio");
        return -1;
    }
    return status;
}

// Funzione per eseguire un comando con input redirection
int execute_command_with_input_redirection(const char *command, const char *input_file) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Errore nella creazione del processo figlio");
        return -1;
    } else if (pid == 0) {
        FILE *file = freopen(input_file, "r", stdin);
        if (file == NULL) {
            perror("Errore nell'apertura del file di input");
            exit(EXIT_FAILURE);
        }
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        perror("Errore nell'esecuzione del comando");
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Errore nell'attesa del processo figlio");
        return -1;
    }
    return status;
}

// Funzione per eseguire un comando con input e output redirection
int execute_command_with_io_redirection(const char *command, const char *input_file, const char *output_file) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Errore nella creazione del processo figlio");
        return -1;
    } else if (pid == 0) {
        FILE *input = freopen(input_file, "r", stdin);
        if (input == NULL) {
            perror("Errore nell'apertura del file di input");
            exit(EXIT_FAILURE);
        }
        FILE *output = freopen(output_file, "w", stdout);
        if (output == NULL) {
            perror("Errore nell'apertura del file di output");
            exit(EXIT_FAILURE);
        }
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        perror("Errore nell'esecuzione del comando");
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Errore nell'attesa del processo figlio");
        return -1;
    }
    return status;
}

// Funzione per ottenere l'ID del processo corrente
pid_t get_current_pid() {
    return getpid();
}

// Funzione per ottenere l'ID del processo padre
pid_t get_parent_pid() {
    return getppid();
}

// Funzione per sospendere l'esecuzione per un determinato numero di secondi
void sleep_for_seconds(unsigned int seconds) {
    sleep(seconds);
}

// Funzione per sospendere l'esecuzione per un determinato numero di millisecondi
void sleep_for_milliseconds(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}

// Funzione per cambiare la directory corrente
int change_directory(const char *path) {
    if (chdir(path) == -1) {
        perror("Errore nel cambio di directory");
        return -1;
    }
    return 0;
}

// Funzione per ottenere la directory corrente
char* get_current_directory() {
    char *buffer = malloc(PATH_MAX);
    if (buffer == NULL) {
        perror("Errore nell'allocazione della memoria");
        return NULL;
    }

    if (getcwd(buffer, PATH_MAX) == NULL) {
        perror("Errore nell'ottenimento della directory corrente");
        free(buffer);
        return NULL;
    }

    return buffer;
}

// Funzione per eseguire un comando come superutente
int execute_command_as_root(const char *command) {
    if (geteuid() != 0) {
        fprintf(stderr, "Errore: questo comando deve essere eseguito come superutente\n");
        return -1;
    }
    return execute_command(command);
}

// Funzione per terminare un processo con un segnale
int terminate_process(pid_t pid, int signal) {
    if (kill(pid, signal) == -1) {
        perror("Errore nell'invio del segnale al processo");
        return -1;
    }
    return 0;
}

int main() {
    // Esempio di utilizzo delle funzioni

    // Eseguire un comando di sistema
    execute_command("echo 'Ciao, mondo!'");

    // Eseguire un comando e catturare l'output
    char *output = execute_command_capture_output("ls");
    if (output != NULL) {
        printf("Output del comando 'ls':\n%s\n", output);
        free(output);
    }

    // Lanciare un processo figlio
    pid_t pid = launch_process("sleep 5");
    if (pid > 0) {
        printf("Processo figlio lanciato con PID %d\n", pid);
        wait_for_process(pid);
    }

    // Ottenere l'ID del processo corrente
    pid_t current_pid = get_current_pid();
    printf("ID del processo corrente: %d\n", current_pid);

    // Ottenere l'ID del processo padre
    pid_t parent_pid = get_parent_pid();
    printf("ID del processo padre: %d\n", parent_pid);

    // Cambiare la directory corrente
    if (change_directory("/") == 0) {
        printf("Directory corrente cambiata con successo.\n");
    }

    // Ottenere la directory corrente
    char *current_directory = get_current_directory();
    if (current_directory != NULL) {
        printf("Directory corrente: %s\n", current_directory);
        free(current_directory);
    }

    return 0;
}
