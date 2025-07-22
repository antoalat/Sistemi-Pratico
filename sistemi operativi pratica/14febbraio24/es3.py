#!/usr/bin/env python3
import sys
import os
import subprocess

def get_file_type(path):
    result = subprocess.run(["file", "-b", path], capture_output=True, text=True)
    if result.returncode == 0:
        return result.stdout.strip()
    else:
        return "Unknown"

def main():
    dir_path = sys.argv[1] if len(sys.argv) > 1 else "."
    
    if not os.path.isdir(dir_path):
        print(f"{dir_path} non è una directory valida")
        sys.exit(1)

    entries = os.listdir(dir_path)
    catalog = {}

    for entry in entries:
        full_path = os.path.join(dir_path, entry)
        ftype = get_file_type(full_path)
        catalog.setdefault(ftype, []).append(entry)

    for ftype in sorted(catalog.keys()):
        print(f"{ftype}:")  
        for name in sorted(catalog[ftype]):
            print(f" {name}")
        print()

if __name__ == "__main__":
    main()
