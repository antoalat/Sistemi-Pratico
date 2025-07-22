import os
import sys
from collections import defaultdict

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Uso: {sys.argv[0]} <directory>")
        sys.exit(1)

    root_dir = sys.argv[1]
    if not os.path.isdir(root_dir):
        print("Errore: directory non valida")
        sys.exit(1)

    inode_map = defaultdict(list)

    for root, dirs, files in os.walk(root_dir):
        for name in dirs + files:
            full_path = os.path.join(root, name)
            if os.path.islink(full_path):
                try:
                    target_path = os.path.realpath(full_path)
                    inode = os.stat(target_path).st_ino
                    inode_map[inode].append(full_path)
                except FileNotFoundError:
                    # Link dangling
                    continue
                except PermissionError:
                    continue

    for inode, links in inode_map.items():
        if len(links) > 1:
            print(f"Link simbolici che puntano allo stesso inode ({inode}):")
            for link in links:
                print(f"  {link}")
            print()
