import os
import sys
from datetime import datetime

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Errore. Uso: {sys.argv[0]} <pattern>")
        sys.exit(1)

    pattern = sys.argv[1].encode()  # converto in bytes per il confronto
    matchs = []

    cwd = os.getcwd()
    for root, dirs, files in os.walk(cwd):
        for file in files:
            full_path = os.path.join(root, file)
            try:
                with open(full_path, "rb") as f:
                    content = f.read()
                    if pattern in content:
                        timestamp = os.stat(full_path).st_mtime
                        matchs.append((full_path, timestamp))
            except (PermissionError, OSError):
                # Salta file non leggibili
                continue

    # Ordina dal più recente al meno recente
    matchs.sort(key=lambda x: x[1], reverse=True)

    for path, timestamp in matchs:
        data_ora = datetime.fromtimestamp(timestamp)
        print(f"Path: {path} | Ultima modifica: {data_ora}")
