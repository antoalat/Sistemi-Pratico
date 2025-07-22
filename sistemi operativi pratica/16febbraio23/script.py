'''
Sia data una directory che contiene file di testo.
Scopo dell'esercizio e' di scrivere un programma Python o uno script bash chiamato ccpl che conti i
caratteri delle corrispondenti righe di testo di tutti i file della directory, si vuole cioe' sapere il numero
totale di caratteri presenti nelle prime righe di tutti i file, nelle seconde linee, ecc.
$ ccpl mydir
1 234
2 21
3 333
…..
l'ouput significa che se contiamo tutti i caratteri contenuti nella prima riga di tutti i file in mydir
otteniamo 234 (mydir/file1 puo' avere 40 caratteri nella prima riga, mydir/file2 ne puo' avere 20, ecc...
procedendo per tutti i file di mydir la somma fa 234).'''

import os
import sys




if __name__ == "__main__":
    if len(sys.argv) !=  2 :
        raise Exception(f"Uso: {sys.argv[0]} <directory>")
    
    if( not os.path.isdir(sys.argv[1])):
        raise Exception(f"Inserire una directory valida. Uso: {sys.argv[0]} <directory>")
    
    d = []
    dir = sys.argv[1]
    for file in os.listdir(dir):
            full_path = os.path.join(dir,file)
            if os.path.isfile(full_path) and file.endswith(".txt"):
                with open(full_path, "r") as f:
                     lines = f.readlines()
                     for index,line in enumerate(lines):
                         if index >= len(d):
                            d.append(0)
                            d[index]+=len(line)
    
    print("Elenco righe file di testo")
    for index, line in enumerate(d):
        print(f"riga {index + 1} : {line} ")