#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

// Funzione per creare una directory
int create_directory(const char *path) {
    if (mkdir(path, 0777) == -1) {
        perror("Errore nella creazione della directory");
        return -1;
    }
    return 0;
}

// Funzione per rimuovere una directory
int remove_directory(const char *path) {
    if (rmdir(path) == -1) {
        perror("Errore nella rimozione della directory");
        return -1;
    }
    return 0;
}

// Funzione per leggere il contenuto di una directory e restituire un array di stringhe
char** read_directory(const char *path, int *count) {
    DIR *dir;
    struct dirent *entry;
    int array_size = 10;
    char **files = malloc(array_size * sizeof(char*));
    if (files == NULL) {
        perror("Errore nell'allocazione della memoria");
        return NULL;
    }

    if ((dir = opendir(path)) == NULL) {
        perror("Errore nell'apertura della directory");
        free(files);
        return NULL;
    }

    *count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        files[*count] = malloc((strlen(entry->d_name) + 1) * sizeof(char));
        if (files[*count] == NULL) {
            perror("Errore nell'allocazione della memoria");
            for (int i = 0; i < *count; i++) {
                free(files[i]);
            }
            free(files);
            closedir(dir);
            return NULL;
        }
        strcpy(files[*count], entry->d_name);
        (*count)++;

        if (*count >= array_size) {
            array_size *= 2;
            char **new_files = realloc(files, array_size * sizeof(char*));
            if (new_files == NULL) {
                perror("Errore nel ridimensionamento dell'array");
                for (int i = 0; i < *count; i++) {
                    free(files[i]);
                }
                free(files);
                closedir(dir);
                return NULL;
            }
            files = new_files;
        }
    }
    closedir(dir);
    return files;
}

// Funzione per liberare la memoria allocata per l'array di stringhe
void free_string_array(char **array, int count) {
    for (int i = 0; i < count; i++) {
        free(array[i]);
    }
    free(array);
}

int main() {
    // Creazione di una directory
    const char *dir_name = "test_dir";
    if (create_directory(dir_name) == 0) {
        printf("Directory '%s' creata con successo.\n", dir_name);
    }

    // Lettura del contenuto di una directory
    int file_count = 0;
    char **files = read_directory(".", &file_count);
    if (files != NULL) {
        printf("Contenuto della directory corrente:\n");
        for (int i = 0; i < file_count; i++) {
            printf("%s\n", files[i]);
        }
        free_string_array(files, file_count);
    }

    // Rimozione di una directory
    if (remove_directory(dir_name) == 0) {
        printf("Directory '%s' rimossa con successo.\n", dir_name);
    }

    return 0;
}
