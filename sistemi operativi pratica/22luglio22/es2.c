#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Funzione ricorsiva per stampare l'albero delle directory
void tree(int dfd, int tabs) {
  // Converte il file descriptor in DIR* per poter usare readdir
  DIR *dir = fdopendir(dfd);
  if (dir == NULL) {
    perror("fdopendir");
    close(dfd);  // Chiudi il file descriptor se fdopendir fallisce
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    // Salta "." e ".."
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      // Stampa l'indentazione in base alla profondità
      for (int i = 0; i < tabs; i++) {
        printf("  ");
      }
      // Stampa il nome dell'entry
      printf("%s\n", entry->d_name);

      // Prova ad aprire l'entry come directory (se non è, fallisce)
      int fd = openat(dfd, entry->d_name, O_RDONLY | O_DIRECTORY);
      if (fd == -1) {
        // Se non è directory o non accessibile, salta
        continue;
      }

      // Usa fstatat per determinare il tipo di file (senza seguire symlink)
      struct stat st;
      if (fstatat(dfd, entry->d_name, &st, AT_SYMLINK_NOFOLLOW) == -1) {
        perror("fstatat");
        close(fd);
        continue;
      }

      // Se è una directory vera, ricorsione
      if (S_ISDIR(st.st_mode)) {
        tree(fd, tabs + 1);
      } else {
        // Altrimenti chiudi solo
        close(fd);
      }
    }
  }

  // Chiude DIR* (e chiude anche automaticamente il file descriptor)
  closedir(dir);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Uso: %s <directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Apre la directory iniziale come file descriptor
  int dfd = open(argv[1], O_RDONLY | O_DIRECTORY);
  if (dfd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  // Stampa il nome iniziale e avvia la ricorsione
  printf("%s\n", argv[1]);
  tree(dfd, 0);

  // Chiusura opzionale (closedir del primo tree ha già chiuso)
  close(dfd);

  return EXIT_SUCCESS;
}
