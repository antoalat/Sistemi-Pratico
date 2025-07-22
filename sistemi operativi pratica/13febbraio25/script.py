import os
import sys

def print_by_size(file_path, dir_path):
    try:
        ref_stat = os.stat(file_path)
    except FileNotFoundError:
        print(f"Errore: il file {file_path} non esiste.")
        return

    ref_size = ref_stat.st_size
    ref_inode = ref_stat.st_ino
    ref_dev = ref_stat.st_dev

    for root, _, files in os.walk(dir_path):
        for name in files:
            full_path = os.path.join(root, name)
            try:
                stat = os.stat(full_path)
                if stat.st_size == ref_size:
                    # escludi i veri hard link (stesso inode e stesso device)
                    if stat.st_ino != ref_inode or stat.st_dev != ref_dev:
                        print(full_path)
            except Exception as e:
                print(f"Errore su {full_path}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Uso: {sys.argv[0]} <file_path> <dir_path>")
        sys.exit(1)

    print("File con stessa ampiezza, ma non hard link del file di riferimento:\n")
    print_by_size(sys.argv[1], sys.argv[2])
