#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define MAX_LEN 8

char buffer[MAX_LEN + 1];
char message[1000]; // buffer più grande per messaggio totale
int idx = 0;
int msg_idx = 0;

void handler(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        int ch = info->si_value.sival_int;
        if (idx < MAX_LEN) {
            buffer[idx++] = (char)ch;
        }
    } else if (sig == SIGUSR2) {
        // Fine blocco
        buffer[idx] = '\0';
        // Copia blocco ricevuto nel messaggio totale
        strncpy(message + msg_idx, buffer, idx);
        msg_idx += idx;
        idx = 0; // reset buffer blocco
    } else if (sig == SIGTERM) {
        // Fine messaggio
        message[msg_idx] = '\0';
        printf("Messaggio ricevuto completo: %s\n", message);
        msg_idx = 0; // reset per prossimo messaggio
    }
}

int main() {
    printf("PID di rx: %d\n", getpid());

    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        return 1;
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction SIGUSR2");
        return 1;
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        return 1;
    }

    while (1) {
        pause();
    }

    return 0;
}
