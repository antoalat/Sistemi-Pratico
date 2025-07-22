import os
import sys

def slinout(root):
    abs_root = os.path.abspath(root)

    for dirpath, dirnames, filenames in os.walk(abs_root):
        entries = dirnames + filenames
        for name in entries:
            path = os.path.join(dirpath, name)

            if os.path.islink(path):
                try:
                    # Dove si trova fisicamente il link (assoluto)
                    abs_symlink = os.path.abspath(path)

                    # Dove punta effettivamente il link (target risolto)
                    real_target = os.path.realpath(path)
                    if real_target.startswith(abs_root + os.sep):
                        print(f"[interno] {abs_symlink} -> {real_target}")
                    else:
                        print(f"[esterno] {abs_symlink} -> {real_target}")
                except OSError:
                    print(f"[rotto] {path} (link non risolto)")

if __name__ == "__main__":
    root = sys.argv[1] if len(sys.argv) > 1 else os.getcwd()
    slinout(root)
