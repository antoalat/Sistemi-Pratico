'''
Scrivere un programma python o uno script bash che, passata una directory come parametro, cancelli
nel sottoalbero generato dalla directory passata come parametro tutti i link simbolici relativi (e non
cancelli quelli assoluti)
lrwxrwxrwx 1 renzo renzo 13 Jun 11 17:03 hostname1 -> /etc/hostname
lrwxrwxrwx 1 renzo renzo 15 Jun 11 17:04 hostname2 -> ../etc/hostname
il primo va mantenuto e il secondo cancellato'''

import os
import sys
import shutil


import os
import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise Exception(f"Uso: {sys.argv[0]} <directory>")
    
    root_dir = sys.argv[1]
    
    if not os.path.isdir(root_dir):
        raise Exception("Errore: directory non valida")
    
    to_remove = []
    
    for root, dirs, files in os.walk(root_dir):
        for entry in dirs + files:
            full_path = os.path.join(root, entry)
            if os.path.islink(full_path):
                target = os.readlink(full_path)
                if target.startswith("/"):
                    continue  # assoluto → lo mantengo
                to_remove.append(full_path)
    
    for path in to_remove:
        try:
            os.remove(path)
            print(f"Rimosso: {path}")
        except FileNotFoundError:
            print(f"File non trovato: {path}")
        except Exception as e:
            print(f"Errore nella rimozione di {path}: {e}")
