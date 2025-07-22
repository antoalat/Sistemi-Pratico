import os
import sys
import subprocess

def execute_scripts(path):
    for file in os.listdir(path):
        full_path = os.path.join(path, file)

        # Skip se non è un file regolare ed eseguibile
        if not os.path.isfile(full_path) or not os.access(full_path, os.X_OK):
            continue

        # Skip se è un binario ELF oppure non è uno script
        try:
            with open(full_path, "rb") as f:
                magic = f.read(4)
                if magic == b'\x7fELF' or magic[:2] != b"#!" :
                    continue
        except Exception as e:
            print(f"Errore leggendo {file}: {e}")
            continue

        # Costruisco il comando in base al tipo di script
        if file.endswith(".py"):
            command = f"python3 '{full_path}'"
        else:
            command = f"'{full_path}'"

        # Esegu il comando
        print(f"Eseguo: {command}")
        result = subprocess.run(command, shell=True)

        if result.returncode != 0:
            print(f"Errore eseguendo: {command}")
            break

if __name__ == "__main__":
    if len(sys.argv) > 2:
        raise Exception(f"Uso: {sys.argv[0]} [?directory]")
    path = os.getcwd() if len(sys.argv) == 1 else sys.argv[1]
    execute_scripts(path)
