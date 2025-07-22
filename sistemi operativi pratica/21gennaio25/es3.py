import os
import sys

def calcola_profondita(path, root):
    # Calcola la profondità relativa di 'path' rispetto a 'root'
    rel_path = os.path.relpath(path, root)
    if rel_path == '.':
        return 0
    return rel_path.count(os.sep) + 1

def stampa_file_ordinati_per_profondita(root_dir):
    elementi = []

    # Esplora la directory in modo ricorsivo
    for dirpath, _, filenames in os.walk(root_dir):
        for nome_file in filenames:
            percorso = os.path.join(dirpath, nome_file)
            prof = calcola_profondita(percorso, root_dir)
            elementi.append((prof, nome_file, percorso))

    # Ordina per profondità e nome
    elementi.sort(key=lambda x: (-x[0], x[1]))

    # Stampa dall'elemento più profondo a quello più vicino alla radice
    for prof, nome_file, percorso in elementi:
        print(f"Profondità: {prof} - Nome: {nome_file} - Percorso: {percorso}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Uso: {sys.argv[0]} <directory>")
        sys.exit(1)

    root = sys.argv[1]
    stampa_file_ordinati_per_profondita(root)
