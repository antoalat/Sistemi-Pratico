import os
import sys
import hashlib

def sha256_of_file(filepath):
    sha256 = hashlib.sha256()
    with open(filepath, 'rb') as f:
        for block in iter(lambda: f.read(4096), b''):
            sha256.update(block)
    return sha256.hexdigest()

def findShaInDir(path, sha):
    found_files = []
    for name in os.listdir(path):
        full_path = os.path.join(path, name)
        if os.path.isfile(full_path):
            try:
                if sha256_of_file(full_path) == sha:
                    found_files.append(full_path)
            except Exception:
                pass
    return found_files

if __name__ == "__main__":  
    if len(sys.argv) != 3:
        raise Exception(f"Uso: {sys.argv[0]} <directory1> <directory2>")

    dir1_path = sys.argv[1]
    dir2_path = sys.argv[2]

    to_remove = []

    for name in os.listdir(dir1_path):
        full_path = os.path.join(dir1_path, name)
        if os.path.isfile(full_path):
            try:
                sha = sha256_of_file(full_path)
                duplicates_in_dir1 = findShaInDir(dir1_path, sha)
                duplicates_in_dir2 = findShaInDir(dir2_path, sha)
                if duplicates_in_dir1 and duplicates_in_dir2:
                    for file in duplicates_in_dir1 + duplicates_in_dir2:
                        if file not in to_remove:
                            to_remove.append(file)
            except Exception as e:
                print(f"Errore leggendo {full_path}: {e}")

    for filepath in to_remove:
        try:
            os.remove(filepath)
            print(f"File: {filepath} eliminato con successo")
        except FileNotFoundError:
            print(f"File: {filepath} non trovato")
        except OSError as e:
            print(f"Errore durante l'eliminazione del file: {e}")
