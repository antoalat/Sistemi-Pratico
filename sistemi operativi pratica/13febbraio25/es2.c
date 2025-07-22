#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/limits.h>

// Funzione che confronta due file byte per byte
int files_are_equal(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;  // se uno dei due non si apre, consideriamo non uguali
    }

    int c1, c2;
    do {
        c1 = fgetc(f1);
        c2 = fgetc(f2);
        if (c1 != c2) {
            fclose(f1);
            fclose(f2);
            return 0;
        }
    } while (c1 != EOF && c2 != EOF);

    fclose(f1);
    fclose(f2);

    return c1 == EOF && c2 == EOF;
}

// Funzione ricorsiva che scorre la directory
void find_equal_files(const char *target_file, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Errore apertura directory");
        return;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    struct stat st;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &st) == -1) {
            perror("Errore stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            find_equal_files(target_file, full_path);
        } else if (S_ISREG(st.st_mode)) {
            if (files_are_equal(target_file, full_path)) {
                printf("%s\n", full_path);
            }
        }
    }
    closedir(dir);
}


int check_by_prefix(int bytes, const char *file_path, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Errore apertura dir");
        return EXIT_FAILURE;
    }

    FILE *f1 = fopen(file_path, "rb");
    if (!f1) {
        perror("Errore apertura file target");
        closedir(dir);
        return EXIT_FAILURE;
    }

    char *buffer = malloc(bytes);
    if (!buffer) {
        perror("malloc");
        fclose(f1);
        closedir(dir);
        return EXIT_FAILURE;
    }

    size_t read1 = fread(buffer, 1, bytes, f1);
    fclose(f1);
    if (read1 < bytes) {
        fprintf(stderr, "Il file target è più corto di %d byte\n", bytes);
        free(buffer);
        closedir(dir);
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    struct stat st;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            check_by_prefix(bytes, file_path, full_path);
        } else if (S_ISREG(st.st_mode)) {
            if (strcmp(full_path, file_path) == 0)
                continue; // salta se è il file target stesso

            FILE *f2 = fopen(full_path, "rb");
            if (!f2)
                continue;

            char *buffer_temp = malloc(bytes);
            if (!buffer_temp) {
                fclose(f2);
                continue;
            }

            size_t read2 = fread(buffer_temp, 1, bytes, f2);
            fclose(f2);

            if (read2 == bytes && memcmp(buffer, buffer_temp, bytes) == 0) {
                printf("%s\n", full_path);
            }

            free(buffer_temp);
        }   
    }

    free(buffer);
    closedir(dir);
    return EXIT_SUCCESS;
}




int main(int argc, char **argv) {
    if (argc == 3) {
        find_equal_files(argv[1], argv[2]);
    } else if (argc == 5 && strcmp(argv[1], "-p") == 0) {
        int n = atoi(argv[2]);
        if (n <= 0) {
            fprintf(stderr, "Numero di byte non valido\n");
            return EXIT_FAILURE;
        }
        return check_by_prefix(n, argv[3], argv[4]);
    } else {
        fprintf(stderr, "Uso:\n");
        fprintf(stderr, "  %s <file> <directory>\n", argv[0]);
        fprintf(stderr, "  %s -p <nbyte> <file> <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
