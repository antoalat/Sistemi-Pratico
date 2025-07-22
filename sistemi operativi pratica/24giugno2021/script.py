'''Scrivere uno script o un programma python che corregga l'indentazione di tutti i file .c e .h presenti
nel sottoalbero della directory passata come parametro (la working directory se non vi sono
parametri).
Hint: il comando:
ex -n '+norm!gg=G' +wq prog.c
corrregge l'indentazione del programma sorgente C prog.c.
'''

import os
import sys
import subprocess

if __name__ == "__main__":
    if len(sys.argv) > 2:
        raise Exception(f"Uso: {sys.argv[0]} <?directory>")
    
    if len(sys.argv) == 2:
        dir = sys.argv[1]
    else:
        dir = os.getcwd()

    for root, dirs, files in os.walk(dir):
        for file in files:
            name, ext = os.path.splitext(file)
            if ext in [".c", ".h"]:
                full_path = os.path.join(root, file)
                try:
                    subprocess.run(["ex", "-n", "+norm!gg=G", "+wq", full_path], capture_output=True, check=True)
                    print(f"Indentazione corretta per: {full_path}")
                except subprocess.CalledProcessError as e:
                    print(f"Errore nella formattazione di {full_path}: {e.stderr.decode()}")

        