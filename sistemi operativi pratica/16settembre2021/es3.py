import os
import sys
from collections import defaultdict

if __name__ == "__main__":
    if len(sys.argv) != 2:
        # È buona norma stampare un errore e uscire, invece di sollevare un'eccezione
        print(f"Uso: {sys.argv[0]} <directory>")
        sys.exit(1)
    
    start_dir = sys.argv[1]
    if not os.path.isdir(start_dir):
        print(f"Errore: La directory '{start_dir}' non esiste.")
        sys.exit(1)

    files_locations = defaultdict(list)

    for root, dirs, files in os.walk(start_dir):
        for file_name in files:
            # --- QUESTA È LA RIGA CORRETTA ---
            # Calcola il percorso della cartella (root) rispetto alla cartella di partenza (start_dir)
            relative_dir = os.path.relpath(root, start_dir)
            files_locations[file_name].append(relative_dir)
    
    # Ordina per nome del file
    for file_name, paths in sorted(files_locations.items()):
        # Ordina anche i percorsi e uniscili con uno spazio per il formato richiesto
        # Esempio: "ciao: . ./a"
        print(f"{file_name}: {' '.join(sorted(paths))}")