import os
import shutil

if __name__ == "__main__":
    cwd = os.getcwd()
    target_dir = os.path.join(cwd, "...")

    os.makedirs(target_dir, exist_ok=True)

    for entry in os.listdir(cwd):
        full_entry_path = os.path.join(cwd, entry)

        if entry == "..." or not os.path.isfile(full_entry_path) or os.path.islink(full_entry_path):
            continue

        new_path = os.path.join(target_dir, entry)

        try:
            # 1. Copia il file nella sottodirectory
            shutil.copy2(full_entry_path, new_path)

            # 2. Crea il link simbolico relativo
            rel_path = os.path.relpath(new_path, cwd)
            os.symlink(rel_path, full_entry_path)

            # 3. Elimina il file originale SOLO DOPO che il link è stato creato
            os.remove(full_entry_path)

        except Exception as e:
            print(f"Errore con file {entry}: {e}")



