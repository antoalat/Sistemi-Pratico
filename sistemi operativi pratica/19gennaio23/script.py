'''Scrivere uno script bash o python che faccia il merge di due alberi del file system copiandoli in un terzo.
La gerarchia risultante dovrebbe contenere tutti i file e le directory presenti nel primo o nel secondo albero.
Se due file hanno lo stesso percorso e nomi uguali nei due alberi di partenza i contenuti devono essere
concatenati nel file risultante'''

import os
import sys
import shutil

def merge_trees(dir1, dir2, dest_dir):
    """
    Esegue il merge di due alberi di directory (dir1, dir2) in una terza (dest_dir).
    """
    # --- Validazione degli input ---
    if not os.path.isdir(dir1):
        print(f"Errore: La directory sorgente '{dir1}' non esiste.", file=sys.stderr)
        sys.exit(1)
    if not os.path.isdir(dir2):
        print(f"Errore: La directory sorgente '{dir2}' non esiste.", file=sys.stderr)
        sys.exit(1)
    
    # Crea la directory di destinazione se non esiste
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    # --- Passaggio 1: Copia l'intero albero di dir1 in dest_dir ---
    for src_root, dirs, files in os.walk(dir1):
        dest_root = os.path.join(dest_dir, os.path.relpath(src_root, dir1))
        for d in dirs:
            dest_path = os.path.join(dest_root, d)
            if not os.path.exists(dest_path):
                os.makedirs(dest_path)
        for f in files:
            src_path = os.path.join(src_root, f)
            dest_path = os.path.join(dest_root, f)
            shutil.copy2(src_path, dest_path)

    # --- Passaggio 2: Esegue il merge di dir2 in dest_dir ---
    for src_root, dirs, files in os.walk(dir2):
        dest_root = os.path.join(dest_dir, os.path.relpath(src_root, dir2))
        for d in dirs:
            dest_path = os.path.join(dest_root, d)
            if not os.path.exists(dest_path):
                os.makedirs(dest_path)
        for f in files:
            src_path = os.path.join(src_root, f)
            dest_path = os.path.join(dest_root, f)

            if os.path.exists(dest_path) and os.path.isfile(dest_path):
                # Il file esiste già: concatena il contenuto in modalità binaria.
                with open(dest_path, 'ab') as f_dest, open(src_path, 'rb') as f_src:
                    f_dest.write(f_src.read())
            else:
                # Il file non esiste nella destinazione: copialo.
                shutil.copy2(src_path, dest_path)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"Uso: {sys.argv[0]} <directory_sorgente_1> <directory_sorgente_2> <directory_destinazione>", file=sys.stderr)
        sys.exit(1)
    
    source_dir1 = sys.argv[1]
    source_dir2 = sys.argv[2]
    destination_dir = sys.argv[3]

    merge_trees(source_dir1, source_dir2, destination_dir)