/* Esercizio 1: Linguaggio C (obbligatorio) 20 punti.
   Usando il metodo inotify implementare il programma dircat.
   dircat ha 2 parametri: il pathname di una directory (vuota) D e di un file F.
   Ogni volta che un file viene messo in D il programma dircat aggiunge a F una riga di testata
   contenente il nome del file e ne copia il contenuto in F. Finita la copia il file che era stato messo in D
   viene cancellato (la directory D torna vuota).
   (per fare gli esperimenti si consiglia di preparare i file di prova in un'altra directory e copiarli in D)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define BUF_LEN 1024
#define LINE_SIZE 4096

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Uso: %s <directory> <file_output>\n", argv[0]);
		exit(1);
	}

	const char *dir = argv[1];
	const char *outfile = argv[2];

	// Controlla se la directory è vuota
	DIR *dp = opendir(dir);
	if (!dp) {
		perror("opendir");
		exit(1);
	}

	struct dirent *ep;
	while ((ep = readdir(dp)) != NULL) {
		if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
			fprintf(stderr, "Errore: la directory non è vuota.\n");
			closedir(dp);
			exit(1);
		}
	}
	closedir(dp);

	// Inizializza inotify
	int fd = inotify_init();
	if (fd < 0) {
		perror("inotify_init");
		exit(1);
	}

	int wd = inotify_add_watch(fd, dir, IN_CREATE | IN_MOVED_TO);
	if (wd < 0) {
		perror("inotify_add_watch");
		exit(1);
	}

	printf("Aspetto nuovi file in %s...\n", dir);

	char buf[BUF_LEN];
	while (1) {
		int len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			perror("read");
			continue;
		}

		int i = 0;
		while (i < len) {
			struct inotify_event *e = (struct inotify_event *) &buf[i];

			if (e->len > 0 && (e->mask & (IN_CREATE | IN_MOVED_TO))) {
				char filepath[PATH_MAX];
				snprintf(filepath, sizeof(filepath), "%s/%s", dir, e->name);

				FILE *src = fopen(filepath, "r");
				if (!src) {
					perror("fopen src");
					i += sizeof(struct inotify_event) + e->len;
					continue;
				}

				FILE *dst = fopen(outfile, "a");
				if (!dst) {
					perror("fopen dst");
					fclose(src);
					i += sizeof(struct inotify_event) + e->len;
					continue;
				}

				fprintf(dst, "--- %s ---\n", e->name);

				char line[LINE_SIZE];
				while (fgets(line, sizeof(line), src)) {
					fputs(line, dst);
				}

				fclose(src);
				fclose(dst);

				if (unlink(filepath) != 0)
					perror("unlink");
				else
					printf("File %s copiato e rimosso.\n", e->name);
			}

			i += sizeof(struct inotify_event) + e->len;
		}
	}

	close(fd);
	return 0;
}
