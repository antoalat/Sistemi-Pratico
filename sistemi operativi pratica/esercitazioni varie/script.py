import sys
import os

def is_elf(filepath):
    """Restituisce True se il file è ELF."""
    try:
        with open(filepath, 'rb') as f:
            return f.read(4) == b'\x7fELF'
    except:
        return False

def calculate_bytes(directories):
    byte_counter = 0
    for directory in directories:
        try:
            for nome_file in os.listdir(directory):
                percorso_completo = os.path.join(directory, nome_file)
                if os.path.isfile(percorso_completo) and os.access(percorso_completo, os.X_OK):
                    if is_elf(percorso_completo):
                        byte_counter += os.path.getsize(percorso_completo)
        except Exception as e:
            # Directory non leggibile o altro errore
            continue
    print(byte_counter)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        calculate_bytes(sys.argv[1:])
    else:
        calculate_bytes([os.getcwd()])
