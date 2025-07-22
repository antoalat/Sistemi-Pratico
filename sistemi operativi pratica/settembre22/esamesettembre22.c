#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s comando [arg1 arg2 ...]\n", argv[0]);
        return 1;
    }

    while (1) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork fallita");
            return 1;
        }

        if (pid == 0) {
            // Processo figlio: esegue il comando
            execvp(argv[1], &argv[1]);
            perror("execvp fallita");
            exit(1); // Se exec fallisce, exit(1)
        }

        // Processo padre: attende il figlio
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid fallita");
            return 1;
        }

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code == 0) {
                printf("Comando terminato con successo. Rilancio...\n");
                continue; // Ripete il ciclo
            } else {
                printf("Comando terminato con codice %d. Interrompo.\n", code);
                break;
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("Comando terminato a causa del segnale %d. Interrompo.\n", sig);
            break;
        } else {
            printf("Comando terminato in modo sconosciuto. Interrompo.\n");
            break;
        }
    }

    return 0;
}


