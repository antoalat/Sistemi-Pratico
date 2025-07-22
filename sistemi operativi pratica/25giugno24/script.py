'''
Scrivere un programma python o uno script bash chiamato permdir. permdir prende come
parametro il pathname di una directory e crea nella directory corrente una directory per stringa di
permessi contenente link simbolici ai file con tali permessi.
es: se /tmp/dir è la directory passata come parametro e:
ls -l /tmp/dir
-rw-r--r-- 1 renzo renzo 0 Jun 20 13:23 due
-rw-r----- 1 renzo renzo 0 Jun 20 13:23 quattro
-rwx------ 1 renzo renzo 0 Jun 20 13:23 tre
-rwx------ 1 renzo renzo 0 Jun 20 13:23 uno
il comando permdir deve creare tre directory:
-rw-r--r-- che contiene il link due che punta a /tmp/dir/due
-rw-r----- che contiene il link quattro che punta a /tmp/dir/quattro
-rwx------ che contiene due link uno e tre che puntano agli omonimi file in /tmp/dir
(gestire solo il caso di file, no directory o file speciali)
'''

import os
import sys
import stat
from collections import defaultdict

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise Exception(f"Uso: {sys.argv[0]} <directory>")

    source_dir = sys.argv[1]
    files_by_perm = defaultdict(list)

    for entry in os.listdir(source_dir):
        full_path = os.path.join(source_dir, entry)
        if os.path.isfile(full_path):
            st = os.stat(full_path)
            perm_str = stat.filemode(st.st_mode)
            full_path = os.path.abspath(full_path)
            files_by_perm[perm_str].append(full_path)

    cwd = os.getcwd()
    for perm, files in files_by_perm.items():
        target_dir = os.path.join(cwd, perm)
        os.makedirs(target_dir, exist_ok=True)
        for file in files:
            link_name = os.path.join(target_dir, os.path.basename(file))
            try:
                os.symlink(file, link_name)
            except FileExistsError:
                pass  
