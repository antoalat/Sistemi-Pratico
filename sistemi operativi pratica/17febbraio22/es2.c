#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

int main(int argc, char **argv)
{
    // 1. Controlla che sia stato passato esattamente un argomento (il percorso della directory)
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <directory_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *dir = argv[1];
    // 2. Prova ad aprire la directory specificata
    DIR *dp = opendir(dir);
    if (!dp)
    {
        perror("Errore apertura dir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char src_path[PATH_MAX];

    // 3. Leggi ogni elemento (file, directory, link, ecc.) nella directory
    while ((entry = readdir(dp)) != NULL)
    {
        // Salta le directory speciali "." (corrente) e ".." (padre)
        if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
            continue;

        // Costruisce il percorso completo dell'elemento (es. "mia_dir/mio_file")
        snprintf(src_path, sizeof(src_path), "%s/%s", dir, entry->d_name);

        struct stat st;
        // 4. Usa lstat() per ottenere informazioni sull'elemento.
        // lstat() è fondamentale perché NON segue il link, ma analizza il link stesso.
        if (lstat(src_path, &st) == -1)
        {
            perror("Errore lettura statistiche sul file");
            continue;
        }

        // 5. Controlla se l'elemento è un link simbolico
        if (S_ISLNK(st.st_mode))
        {
            char target_link[PATH_MAX];
            // Leggi la destinazione (il target) del link simbolico
            ssize_t len = readlink(src_path, target_link, sizeof(target_link) - 1);
            if (len == -1)
            {
                perror("Errore lettura link");
                continue;
            }
            target_link[len] = '\0'; // Aggiungi il terminatore di stringa

            // 6. Costruisci il percorso completo del target del link
            char final_target[PATH_MAX];
            if (target_link[0] == '/') // Se il link punta a un percorso assoluto
            {
                snprintf(final_target, sizeof(final_target), "%s", target_link);
            }
            else // Se il link punta a un percorso relativo
            {
                char temp[PATH_MAX];
                strncpy(temp, src_path, sizeof(temp));
                // Ottieni la directory del link per risolvere il percorso relativo
                char *link_dir = dirname(temp);
                snprintf(final_target, sizeof(final_target), "%s/%s", link_dir, target_link);
            }

            struct stat target_stat;
            // 7. Usa lstat() ANCHE sul target del link
            if (lstat(final_target, &target_stat) == -1)
            {
                // Questo errore può accadere se il link è "rotto" (punta a un file che non esiste)
                // perror("Errore lstat target"); // Puoi decommentare per debug
                continue;
            }

            // 8. Se anche il target è un link simbolico, abbiamo trovato un link NIDIFICATO
            if (S_ISLNK(target_stat.st_mode))
            {
                char resolved_final[PATH_MAX];
                // Usa realpath() per risolvere l'intera catena di link e trovare il file REALE finale
                if (realpath(final_target, resolved_final) == NULL)
                {
                    perror("Errore realpath (forse il link finale è rotto?)");
                    continue;
                }

                // 9. Sostituzione: prima rimuovi il link simbolico originale...
                if (unlink(src_path) == -1)
                {
                    perror("Errore unlink");
                    continue;
                }

                // ...poi crea un hard link al file reale con lo stesso nome del link originale
                if (link(resolved_final, src_path) == -1)
                {
                    perror("Errore creazione hard link");
                    continue;
                }

                printf("Sostituito link simbolico nidificato %s con hard link a %s\n", src_path, resolved_final);
            }
        }
    }

    closedir(dp);
    return EXIT_SUCCESS;
}