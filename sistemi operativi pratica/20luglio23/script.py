'''Esercizio 3: Python o bash: 10 punti
Scrivere un programma python o uno script bash che data una directory produca un elenco dei file e
delle directory che non potrebbero essere copiati in file system che supportino solo caratteri ascii nei
nomi
'''


import os
import sys

def is_ascii(s):
    return all(ord(c) < 128 for c in s)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Uso: {sys.argv[0]} <directory>")
        sys.exit(1)

    directory = sys.argv[1]

    if not os.path.isdir(directory):
        print(f"Errore: '{directory}' non è una directory valida.")
        sys.exit(1)

    print("Elenco di file e directory con caratteri non ASCII nel nome:")

    for entry in os.listdir(directory):
        if not is_ascii(entry):
            print(entry)
