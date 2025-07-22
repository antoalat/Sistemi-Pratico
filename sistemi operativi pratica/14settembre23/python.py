import os
import sys
from collections import defaultdict

def get_symlink(path):
    symlink_map = defaultdict(list)
    for root, dirs, files in os.walk(path):
        for file_name in files:
            full_path = os.path.join(root, file_name)
            if os.path.islink(full_path):
                try:
                    real_path = os.path.realpath(full_path)
                    symlink_map[real_path].append(full_path)
                except Exception as e:
                    print(f"Errore su {full_path}: {e}")
                    continue
    return symlink_map

if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise Exception("Usage: python3 script.py <directory>")

    symlink_map = get_symlink(sys.argv[1])

    for real_path, links in symlink_map.items():
        if len(links) > 1:
            print(f"File reale: {real_path}")
            for link in links:
                print(f"  {link}")
            print("\n")
