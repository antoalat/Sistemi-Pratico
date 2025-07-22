import os
import sys
import subprocess
from collections import defaultdict

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise Exception(f"Uso: {sys.argv[0]} <directory>")

    directory = sys.argv[1]
    #Se non è una directory lancio un eccezione
    if not os.path.isdir(directory):
        raise Exception(f"{directory} non è una directory valida")

    types_dict = defaultdict(list)

    for entry in os.listdir(directory):
        full_path = os.path.join(directory, entry)

        #Se è una directory o è un file eseguo  il comando file e salvo l'output
        if os.path.isfile(full_path) or os.path.isdir(full_path):
            result = subprocess.run(["file", full_path], capture_output=True, text=True)

            if result.returncode == 0:
                #Se il risultato è 0 rimuovo il carattere finale
                output = result.stdout.strip()
                #Trovo la prima occorrenza del :
                colon_pos = output.find(':')
                if colon_pos != -1:
                    filetype = output[colon_pos+2:]  # tutto dopo ": "
                    types_dict[filetype].append(entry)
            else:
                print(f"Errore durante l'esecuzione di file su {full_path}")

    for filetype, files in types_dict.items():
        print(f"{filetype}: {' '.join(files)}")
