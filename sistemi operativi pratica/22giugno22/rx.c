#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define MAX_LEN 8

char buffer[MAX_LEN+1];
int idx = 0;

void handler(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        int ch = info->si_value.sival_int;
        if (idx < MAX_LEN) {
            buffer[idx++] = (char)ch;
        }
    } else if (sig == SIGUSR2) {
        // Fine stringa
        buffer[idx] = '\0';
        printf("Messaggio ricevuto: %s\n", buffer);
        idx = 0; // reset per prossimo messaggio
        // eventualmente terminare se vuoi: _exit(0);
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

    // Ciclo infinito ad aspettare segnali
    while(1) {
        pause();
    }

    return 0;
}
