#include <stdio.h>      // Funzioni standard di I/O (printf, fprintf, fopen, fclose, fgets, fputs, perror)
#include <stdlib.h>     // Funzioni standard per la gestione della memoria e uscita (exit)
#include <unistd.h>     // Funzioni POSIX (unlink, read, close, pipe, fork, dup2, execl)
#include <string.h>     // Funzioni per manipolazione stringhe (strcmp, snprintf)
#include <fcntl.h>      // Flag per open (non direttamente usato per fopen, ma utile per I/O a basso livello)
#include <sys/inotify.h>// Funzioni inotify (inotify_init, inotify_add_watch)
#include <sys/stat.h>   // Per struct stat, S_ISREG, S_IXUSR, S_IXGRP, S_IXOTH, stat
#include <sys/wait.h>   // Per waitpid, WIFEXITED, WEXITSTATUS (gestione processi figli)
#include <dirent.h>     // Per la gestione delle directory (opendir, readdir, closedir)
#include <errno.h>      // Per errno e perror

#define BUF_LEN 1024    // Dimensione del buffer per leggere gli eventi inotify
#define LINE_SIZE 4096  // Dimensione del buffer per leggere le righe dei file o l'output dei processi

// Funzione: is_executable
// Parametri:
//   - path: il percorso del file da controllare
// Ritorna:
//   - 1 se il file esiste, è un file regolare e ha almeno un permesso di esecuzione (utente, gruppo, altri)
//   - 0 altrimenti (non esiste, non è regolare, non è eseguibile)
int is_executable(const char *path) {
	struct stat st;
	// stat() ottiene le informazioni sullo stato del file.
	// Se fallisce (es. file non trovato o permessi insufficienti), ritorna 0.
	if (stat(path, &st) == -1) {
		return 0;
	}
	// S_ISREG(st.st_mode) controlla se è un file regolare (non una directory, un link, ecc.).
	// (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) controlla se uno qualsiasi dei bit di esecuzione
	// (per utente proprietario, gruppo, o altri) è settato.
	return S_ISREG(st.st_mode) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH));
}

// Funzione: execute_and_capture_output
// Parametri:
//   - filepath: il percorso del file eseguibile da avviare
//   - dst: il puntatore al file (già aperto in modalità scrittura/append) dove scrivere l'output
// Ritorna:
//   - Il codice di uscita del processo figlio se l'esecuzione è avvenuta con successo.
//   - -1 in caso di errore (pipe, fork, fdopen) o se il figlio non è terminato normalmente.
int execute_and_capture_output(const char *filepath, FILE *dst) {
	int pipefd[2]; // Array di due int per i file descriptor della pipe: [0] per lettura, [1] per scrittura

	// pipe() crea una pipe anonima. Restituisce 0 in caso di successo, -1 in caso di errore.
	// pipefd[0] sarà il lato di lettura della pipe, pipefd[1] il lato di scrittura.
	if (pipe(pipefd) == -1) {
		perror("pipe"); // Stampa errore se la pipe non può essere creata
		return -1;
	}

	pid_t pid = fork(); // fork() crea un nuovo processo.
			    // Ritorna:
			    //   - -1 in caso di errore nella creazione del processo figlio.
			    //   - 0 nel processo figlio.
			    //   - Il PID del processo figlio nel processo padre.
	if (pid == -1) {
		perror("fork"); // Stampa errore se la fork fallisce
				// Chiude i file descriptor della pipe per evitare leak se fork fallisce
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}

	if (pid == 0) {
		// Codice Eseguito dal Processo Figlio
		// Il figlio eredita tutti i file descriptor dal padre.
		// Dobbiamo chiudere il lato di lettura della pipe (pipefd[0]) nel figlio,
		// perché il figlio scriverà solo nella pipe.
		close(pipefd[0]);

		// dup2() redirige lo standard output (STDOUT_FILENO) e lo standard error (STDERR_FILENO)
		// del figlio al lato di scrittura della pipe (pipefd[1]).
		// Questo significa che tutto ciò che il figlio scriverà su stdout/stderr andrà nella pipe.
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);

		// Chiude il lato di scrittura della pipe (pipefd[1]) nel figlio,
		// dato che ora stdout/stderr puntano già ad essa.
		close(pipefd[1]);

		// execl() sostituisce l'immagine del processo corrente (il figlio) con un nuovo programma.
		// filepath: il percorso del file eseguibile.
		// filepath (secondo parametro): il primo argomento da passare al programma (argv[0]).
		// NULL: termina la lista degli argomenti.
		// Se execl ha successo, il codice sottostante non viene mai eseguito.
		execl(filepath, filepath, NULL);

		// Se execl ritorna, significa che c'è stato un errore (es. file non trovato, permessi).
		perror("execl"); // Stampa errore se execl fallisce
		exit(1);         // Il figlio esce con un codice di errore
	} else {
		// Codice Eseguito dal Processo Padre
		// Il padre non scriverà nella pipe, quindi chiude il lato di scrittura (pipefd[1]).
		close(pipefd[1]);

		// fdopen() converte un file descriptor (pipefd[0]) in un puntatore FILE*
		// per poter usare funzioni I/O di alto livello (fgets) per leggere dalla pipe.
		FILE *pipe_read = fdopen(pipefd[0], "r");
		if (!pipe_read) {
			perror("fdopen"); // Stampa errore se fdopen fallisce
			close(pipefd[0]); // Chiude il lato di lettura della pipe
			return -1;
		}

		char buffer[LINE_SIZE]; // Buffer per leggere l'output dal processo figlio
					// Legge l'output dal processo figlio (attraverso la pipe) riga per riga
					// e lo scrive nel file di destinazione 'dst'.
		while (fgets(buffer, sizeof(buffer), pipe_read)) {
			fputs(buffer, dst);
		}
		fclose(pipe_read); // Chiude il puntatore FILE* associato alla pipe (chiude anche pipefd[0])

		int status; // Variabile per memorizzare lo stato di terminazione del figlio
			    // waitpid() attende che il processo figlio specificato (pid) termini.
			    // status: verrà riempito con lo stato di terminazione.
			    // 0: nessuna opzione aggiuntiva.
		waitpid(pid, &status, 0);

		// WIFEXITED(status): Macro che restituisce vero se il figlio è terminato normalmente.
		// WEXITSTATUS(status): Macro che restituisce il codice di uscita del figlio (se terminato normalmente).
		return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	}
}


int main(int argc, char *argv[]) {
	// 1. Controllo degli argomenti da riga di comando
	if (argc != 3) {
		fprintf(stderr, "Uso: %s <directory> <file_output>\n", argv[0]);
		return 1; // Esce con errore
	}

	const char *dir = argv[1];      // Directory da monitorare
	const char *outfile = argv[2];  // File di output

	// 2. Controllo iniziale della directory (deve essere vuota)
	// opendir() apre un flusso per la directory.
	DIR *dp = opendir(dir);
	if (!dp) {
		perror("opendir"); // Errore se non è possibile aprire la directory
		return 1;
	}
	struct dirent *ep; // Struttura per memorizzare le voci della directory
			   // readdir() legge la voce successiva nella directory.
	while ((ep = readdir(dp)) != NULL) {
		// strcmp() confronta il nome della voce con "." e "..".
		// Se trova qualsiasi altra voce, la directory non è vuota.
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
			fprintf(stderr, "Errore: la directory non è vuota. Controllare '%s'.\n", ep->d_name);
			closedir(dp); // Chiude il flusso della directory
			return 1;
		}
	}
	closedir(dp); // Chiude il flusso della directory

	// 3. Inizializzazione di inotify
	// inotify_init() crea una nuova istanza inotify.
	int fd = inotify_init();
	if (fd < 0) {
		perror("inotify_init"); // Errore se l'inizializzazione fallisce
		return 1;
	}

	// inotify_add_watch() aggiunge la directory al monitoraggio di inotify.
	// IN_CREATE: notifica quando un file/directory viene creato.
	// IN_MOVED_TO: notifica quando un file/directory viene spostato qui.
	int wd = inotify_add_watch(fd, dir, IN_CREATE | IN_MOVED_TO);
	if (wd < 0) {
		perror("inotify_add_watch"); // Errore se l'aggiunta del watch fallisce
		close(fd); // Chiude il file descriptor inotify
		return 1;
	}

	printf("Aspetto nuovi file in %s...\n", dir); // Messaggio informativo

	char buf[BUF_LEN]; // Buffer per memorizzare gli eventi inotify letti
			   // 4. Ciclo Infinito di Monitoraggio Eventi
	while (1) {
		// read() blocca il programma finché non ci sono eventi inotify da leggere.
		// Legge gli eventi nel buffer 'buf'.
		int len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			perror("read"); // Errore nella lettura degli eventi
			continue; // Continua al ciclo successivo
		}

		// 5. Elaborazione degli Eventi Letti
		// Un singolo 'read' può restituire più eventi inotify concatenati nel buffer.
		int i = 0;
		while (i < len) {
			// Cast del puntatore per interpretare la sezione corrente del buffer come un evento inotify.
			struct inotify_event *e = (struct inotify_event *) &buf[i];

			// Controlla che il nome del file nell'evento non sia vuoto e che l'evento sia di tipo CREATE o MOVED_TO.
			if (e->len > 0 && (e->mask & (IN_CREATE | IN_MOVED_TO))) {
				char filepath[PATH_MAX]; // Buffer per il percorso completo del file
							 // snprintf() costruisce il percorso completo del file (es. "/path/to/D/nomefile.txt").
				snprintf(filepath, sizeof(filepath), "%s/%s", dir, e->name);

				// Apre il file di output in modalità append.
				FILE *dst = fopen(outfile, "a");
				if (!dst) {
					perror("fopen dst"); // Errore se non si riesce ad aprire il file di output
							     // Salta all'inizio del prossimo evento
					i += sizeof(struct inotify_event) + e->len;
					continue;
				}

				// Scrive l'intestazione nel file di output, indicando il file processato.
				fprintf(dst, "--- %s ---\n", e->name);

				// Verifica se il file è eseguibile
				if (is_executable(filepath)) {
					// Se è eseguibile, lo esegue e cattura il suo output.
					printf("Eseguendo file: %s\n", e->name);
					int ret = execute_and_capture_output(filepath, dst);
					// Scrive il codice di uscita del processo nel file di output.
					fprintf(dst, "\n[Processo terminato con codice %d]\n\n", ret);
				} else {
					// Se non è eseguibile, copia semplicemente il suo contenuto.
					printf("Copiando contenuto file: %s\n", e->name);
					FILE *src = fopen(filepath, "r"); // Apre il file sorgente in lettura
					if (!src) {
						perror("fopen src"); // Errore se non si riesce ad aprire il file sorgente
					} else {
						char line[LINE_SIZE]; // Buffer per leggere le righe
								      // Legge riga per riga dal sorgente e scrive nel destinazione.
						while (fgets(line, sizeof(line), src)) {
							fputs(line, dst);
						}
						fclose(src); // Chiude il file sorgente
					}
				}
				fclose(dst); // Chiude il file di output

				// 6. Eliminazione del File dalla Directory Monitorata
				// unlink() cancella il file appena processato.
				if (unlink(filepath) != 0) {
					perror("unlink"); // Errore se la cancellazione fallisce
				} else {
					printf("File %s processato e rimosso.\n", e->name); // Messaggio di conferma
				}
			}

			// Avanza l'indice 'i' per puntare all'inizio del prossimo evento nel buffer.
			// La dimensione di un evento è fissa (sizeof(struct inotify_event)) più la lunghezza
			// variabile del nome del file (e->len).
			i += sizeof(struct inotify_event) + e->len;
		}
	}

	// close(fd) (teoricamente non raggiunto in un ciclo infinito senza break o exit)
	close(fd); // Chiude il file descriptor inotify
	return 0;  // Codice di uscita di successo
}
