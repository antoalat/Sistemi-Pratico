#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Funzione per leggere un file riga per riga
void read_file_line_by_line(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        printf("%s", line);
    }

    free(line);
    fclose(file);
}

// Funzione per scrivere in un file
void write_to_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return;
    }

    fprintf(file, "%s", content);
    fclose(file);
}

// Funzione per appendere contenuto a un file
void append_to_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return;
    }

    fprintf(file, "%s", content);
    fclose(file);
}

// Funzione per ottenere la dimensione di un file
long get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definizione della dimensione iniziale per l'allocazione della memoria
#define INITIAL_ARRAY_SIZE 10
#define LINE_BUFFER_SIZE 1024

// Funzione per leggere tutte le righe di un file e restituire un array bidimensionale
char** read_file_to_array(const char *filename, int *line_count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return NULL;
    }

    // Allocazione iniziale per l'array di stringhe
    int array_size = INITIAL_ARRAY_SIZE;
    char **lines = malloc(array_size * sizeof(char*));
    if (lines == NULL) {
        perror("Errore nell'allocazione della memoria");
        fclose(file);
        return NULL;
    }

    char line[LINE_BUFFER_SIZE];
    *line_count = 0;

    // Lettura del file riga per riga
    while (fgets(line, LINE_BUFFER_SIZE, file)) {
        // Rimozione del carattere di newline, se presente
        line[strcspn(line, "\n")] = '\0';

        // Allocazione della memoria per la riga
        lines[*line_count] = malloc((strlen(line) + 1) * sizeof(char));
        if (lines[*line_count] == NULL) {
            perror("Errore nell'allocazione della memoria");
            // Liberare la memoria già allocata
            for (int i = 0; i < *line_count; i++) {
                free(lines[i]);
            }
            free(lines);
            fclose(file);
            return NULL;
        }
        strcpy(lines[*line_count], line);
        (*line_count)++;

        // Se necessario, ridimensionare l'array di stringhe
        if (*line_count >= array_size) {
            array_size *= 2;
            char **new_lines = realloc(lines, array_size * sizeof(char*));
            if (new_lines == NULL) {
                perror("Errore nel ridimensionamento dell'array");
                // Liberare la memoria già allocata
                for (int i = 0; i < *line_count; i++) {
                    free(lines[i]);
                }
                free(lines);
                fclose(file);
                return NULL;
            }
            lines = new_lines;
        }
    }

    fclose(file);
    return lines;
}

// Funzione per liberare la memoria allocata per l'array bidimensionale
void free_string_array(char **array, int count) {
    for (int i = 0; i < count; i++) {
        free(array[i]);
    }
    free(array);
}

int main() {
    // Esempio di utilizzo delle funzioni

    // Leggere un file riga per riga
    printf("Contenuto del file 'example.txt':\n");
    read_file_line_by_line("example.txt");

    // Scrivere in un file
    const char *content = "Questo è un esempio di contenuto.\n";
    write_to_file("output.txt", content);

    // Appendere contenuto a un file
    const char *additional_content = "Questo è un contenuto aggiuntivo.\n";
    append_to_file("output.txt", additional_content);

    // Ottenere la dimensione di un file
    long size = get_file_size("output.txt");
    if (size != -1) {
        printf("La dimensione del file 'output.txt' è: %ld byte\n", size);
    }

    int line_count;
    char **lines = read_file_to_array("example.txt", &line_count);
    if (lines == NULL) {
        fprintf(stderr, "Errore nella lettura del file\n");
        return 1;
    }

    printf("Contenuto del file example.txt:\n");
    for (int i = 0; i < line_count; i++) {
        printf("%s\n", lines[i]);
    }
    printf("Numero di righe lette: %d\n", line_count);

    // Liberare la memoria allocata
    free_string_array(lines, line_count);

    return 0;
}
