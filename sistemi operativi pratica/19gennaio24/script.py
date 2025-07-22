import os 
import sys

def tcmp(dir1, dir2, param=None):
    d1 = []
    d2 = []

    for root, dirs, files in os.walk(dir1):
        entries = dirs + files
        for entry in entries:
            full_path = os.path.join(root, entry)
            rel_path = os.path.relpath(full_path, dir1)
            d1.append(rel_path)
                
    for root, dirs, files in os.walk(dir2):
        entries = dirs + files
        for entry in entries:
            full_path = os.path.join(root, entry)
            rel_path = os.path.relpath(full_path, dir2)
            d2.append(rel_path)
    
    if param is None:
        print(f"Elenco path comuni a {dir1} e {dir2}")
        for path in sorted(d1):
            if path in d2:
                print(path)
    elif param == "-1":
        print(f"Elenco path che stanno in {dir1} ma non in {dir2}")
        for path in sorted(d1):
            if path not in d2:
                print(path)
    elif param == "-2":
        print(f"Elenco path che stanno in {dir2} ma non in {dir1}")
        for path in sorted(d2):
            if path not in d1:
                print(path)

if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        raise Exception(f"Uso: {sys.argv[0]} [-1|-2] <directory1> <directory2>")

    if len(sys.argv) == 3:
        tcmp(sys.argv[1], sys.argv[2])
    else:
        tcmp(sys.argv[2], sys.argv[3], sys.argv[1])
