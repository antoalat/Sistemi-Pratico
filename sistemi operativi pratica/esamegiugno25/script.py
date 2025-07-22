"""
Questo e' un commento aggiunto automaticamente dallo script.
"""

import os
import sys

# Il testo che vuoi aggiungere come commento
comment_text = "Questo e' un commento aggiunto automaticamente dallo script."

def add_comment_to_file(full_path, content, comment):
    """Sovrascrive il file con il nuovo contenuto commentato."""
    with open(full_path, "w") as f:
        f.write(comment)
        f.write(content)

if __name__ == "__main__":
    cwd = os.getcwd()
    for entry in os.listdir(cwd):
        full_path = os.path.join(cwd, entry)

        if os.path.isfile(full_path):
            # Gestione file Python
            if entry.endswith(".py"):
                with open(full_path, "r") as f:
                    content = f.read()
                # Crea il commento multi-lina per Python
                py_comment = f'"""\n{comment_text}\n"""\n\n'
                add_comment_to_file(full_path, content, py_comment)
                print(f"Aggiunto commento a: {entry}")

            # Gestione file C
            elif entry.endswith(".c"):
                with open(full_path, "r") as f:
                    content = f.read()
                # Crea il commento multi-lina per C
                c_comment = f'/*\n{comment_text}\n*/\n\n'
                add_comment_to_file(full_path, content, c_comment)
                print(f"Aggiunto commento a: {entry}")

            # Gestione file Bash
            elif entry.endswith(".sh"):
                with open(full_path, "r") as f:
                    lines = f.readlines()
                
                # Prepara le linee di commento per Bash
                bash_comment_lines = [f'# {line}\n' for line in comment_text.split('\n')]
                
                with open(full_path, "w") as f:
                    # Se il file non è vuoto e la prima riga è uno shebang
                    if lines and lines[0].startswith("#!"):
                        f.write(lines[0]) # Scrivi la riga shebang
                        f.write("\n")
                        f.writelines(bash_comment_lines) # Scrivi il commento
                        f.writelines(lines[1:]) # Scrivi il resto del contenuto originale
                    else:
                        f.writelines(bash_comment_lines) # Scrivi il commento all'inizio
                        f.writelines(lines) # Scrivi il contenuto originale
                print(f"Aggiunto commento a: {entry}")
                