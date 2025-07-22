/**
 * @file stdn2pip.c
 * @brief Questo programma simula il comportamento di una pipeline della shell (es. cmd1 | cmd2).
 * Legge due comandi dallo standard input, uno per riga, e li esegue in modo che
 * lo standard output del primo comando diventi lo standard input del secondo.
 * 
 * Esempio di input:
 * ls -l
 * grep .c
 * 
 * Il programma eseguirà l'equivalente di `ls -l | grep .c`.
 */

#include <stdio.h>      // Per funzioni di I/O standard (fgets, perror, printf)
#include <stdlib.h>     // Per funzioni di utilità (exit, malloc, free)
#include <unistd.h>     // Per chiamate di sistema POSIX (pipe, fork, dup2, execvp, close)
#include <string.h>     // Per la manipolazione di stringhe (strtok, strcspn)
#include <sys/wait.h>   // Per la funzione waitpid, per attendere la terminazione dei processi figli

// Definiamo delle costanti per evitare di usare "numeri magici" nel codice.
#define MAX_LINE 1024   // Lunghezza massima di una riga di comando letta da input
#define MAX_ARGS 100    // Numero massimo di argomenti per un singolo comando

/**
 * @brief Converte una riga di testo in un array di argomenti (stile argv).
 * 
 * Questa funzione prende una stringa (un comando completo, es. "ls -l -a") e la divide
 * in "token" (parole) usando lo spazio come delimitatore. Il risultato è un array
 * di stringhe che può essere usato direttamente dalla funzione execvp.
 * 
 * @param line La stringa di input da "parsare" (analizzare).
 * @param args L'array di puntatori a stringa che conterrà gli argomenti.
 */
void parse_line(char *line, char **args) {
    // 1. Rimuove il carattere di newline ('\n') che fgets lascia alla fine della stringa.
    //    strcspn trova la prima occorrenza di '\n' e noi la sostituiamo con '\0' (terminatore di stringa).
    line[strcspn(line, "\n")] = '\0';

    // 2. Inizia a "tokenizzare" la stringa. strtok divide la stringa in base ai delimitatori.
    //    La prima chiamata a strtok richiede la stringa da analizzare.
    char *token = strtok(line, " ");
    
    int i = 0;
    // 3. Continua a estrarre token finché ce ne sono e non superiamo il limite di argomenti.
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token; // Salva il puntatore al token nell'array args
        // Le chiamate successive a strtok con NULL gli dicono di continuare a lavorare
        // sulla stessa stringa della chiamata precedente.
        token = strtok(NULL, " ");
    }
    
    // 4. Aggiunge un puntatore NULL alla fine dell'array di argomenti.
    //    Questo è un requisito fondamentale per la famiglia di funzioni execvp,
    //    che usano NULL per sapere dove finisce la lista degli argomenti.
    args[i] = NULL;
}

int main() {
    // Buffer per contenere le righe di testo lette da input
    char line1[MAX_LINE], line2[MAX_LINE];
    // Array di puntatori a stringa per contenere gli argomenti dei due comandi
    char *args1[MAX_ARGS], *args2[MAX_ARGS];

    // --- FASE 1: Lettura e Parsing dei Comandi ---

    printf("Inserisci il primo comando (es. ls -l): ");
    if (!fgets(line1, sizeof(line1), stdin)) {
        perror("Errore nella lettura della prima riga");
        exit(EXIT_FAILURE); // Esce con un codice di errore
    }

    printf("Inserisci il secondo comando (es. grep .c): ");
    if (!fgets(line2, sizeof(line2), stdin)) {
        perror("Errore nella lettura della seconda riga");
        exit(EXIT_FAILURE);
    }

    // Converte le righe di testo in array di argomenti utilizzabili da execvp
    parse_line(line1, args1);
    parse_line(line2, args2);

    // --- FASE 2: Creazione della Pipe ---

    int pipefd[2]; // Array per contenere i file descriptor della pipe
    // pipefd[0] -> estremità di LETTURA della pipe
    // pipefd[1] -> estremità di SCRITTURA della pipe

    if (pipe(pipefd) == -1) {
        perror("Errore nella creazione della pipe");
        exit(EXIT_FAILURE);
    }

    // --- FASE 3: Creazione dei Processi Figli per Eseguire i Comandi ---

    // Creiamo il primo processo figlio, che eseguirà il primo comando
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("Errore nel primo fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // --- Codice eseguito SOLO dal PRIMO FIGLIO ---
        
        // 1. Il primo figlio scriverà nella pipe, quindi non ha bisogno di leggere.
        //    Chiudiamo l'estremità di lettura della pipe.
        close(pipefd[0]);
        
        // 2. Reindirizziamo lo standard output (STDOUT_FILENO, di solito lo schermo)
        //    verso l'estremità di scrittura della pipe (pipefd[1]).
        //    Da ora in poi, tutto ciò che questo processo scriverà su stdout andrà nella pipe.
        dup2(pipefd[1], STDOUT_FILENO);
        
        // 3. Dopo la duplicazione, il file descriptor originale non serve più.
        close(pipefd[1]);
        
        // 4. Eseguiamo il primo comando. execvp sostituisce l'immagine di questo processo
        //    con quella del comando specificato. Se ha successo, il codice sottostante non verrà mai eseguito.
        execvp(args1[0], args1);
        
        // Se execvp fallisce, lo stampiamo e terminiamo.
        perror("Errore exec primo comando");
        exit(EXIT_FAILURE);
    }

    // Creiamo il secondo processo figlio, che eseguirà il secondo comando
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("Errore nel secondo fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        // --- Codice eseguito SOLO dal SECONDO FIGLIO ---
        
        // 1. Il secondo figlio leggerà dalla pipe, quindi non ha bisogno di scrivere.
        //    Chiudiamo l'estremità di scrittura della pipe.
        close(pipefd[1]);
        
        // 2. Reindirizziamo lo standard input (STDIN_FILENO, di solito la tastiera)
        //    per leggere dall'estremità di lettura della pipe (pipefd[0]).
        //    Da ora in poi, tutto ciò che questo processo leggerà da stdin proverrà dalla pipe.
        dup2(pipefd[0], STDIN_FILENO);
        
        // 3. Dopo la duplicazione, il file descriptor originale non serve più.
        close(pipefd[0]);
        
        // 4. Eseguiamo il secondo comando.
        execvp(args2[0], args2);
        
        // Se execvp fallisce, lo stampiamo e terminiamo.
        perror("Errore exec secondo comando");
        exit(EXIT_FAILURE);
    }

    // --- FASE 4: Gestione del Processo Padre ---
    
    // Il processo padre non legge né scrive nella pipe; il suo compito è solo orchestrare.
    // È FONDAMENTALE che il padre chiuda entrambe le estremità della pipe.
    // Se il padre non chiudesse pipefd[1], il secondo figlio non riceverebbe mai EOF (End-Of-File)
    // e rimarrebbe in attesa di input all'infinito.
    close(pipefd[0]);
    close(pipefd[1]);
    
    // Il padre attende la terminazione di entrambi i processi figli per evitare processi "zombie".
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    printf("Pipeline completata.\n");
    return 0; // Il programma termina con successo
}
