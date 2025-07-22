#! /usr/bin/env python3

import subprocess
import os


def get_files_and_directories(path, sottoalbero=True):
    result = []
    if sottoalbero:
        process = subprocess.run(["find", path], capture_output=True, text=True)
    else:
        process = subprocess.run(
            ["find", path, "-maxdepth", "1"], capture_output=True, text=True
        )

    files_and_dirs = process.stdout.splitlines()
    result.extend(files_and_dirs)
    return result


def get_file_type(file_path):
    process = subprocess.run(["file", "-b", file_path], capture_output=True, text=True)
    if process.returncode != 0:
        print(f"Error: {process.stderr}")
        return "Unknown"
    file_type = process.stdout.strip()
    return file_type


def get_file_name(absolute_path):
    return absolute_path.split("/")[-1]


def is_ascii(s):
    for c in s:
        if ord(c) > 127:
            return False
    return True


def is_ELF(file):
    try:
        with open(file, "rb") as f:
            magic = f.read()[:4]
        return magic == b"\x7fELF"
    except Exception as e:
        return False


def is_script(path):
    try:
        with open(path, "rb") as file:
            buffer = file.read()[:2]
        return buffer == b"#!"
    except Exception as e:
        return False


def is_executable(pathname):
    if os.path.isfile(pathname):
        return os.access(pathname, os.X_OK)
    return False


def get_file_size(file_path):
    try:
        with open(file_path, "rb") as file:
            file.seek(0, 2)  # Spostati alla fine del file
            size = (
                file.tell()
            )  # Ottieni la posizione corrente che rappresenta la dimensione del file
        return size
    except IOError as e:
        print(f"Error: {e}")
        return None


# Testo le funzioni del modulo
if __name__ == "__main__":
    path = "/Users/andrea/Desktop/Pratico_SO/Exams2/20240214/testprog"

    print(get_file_size(path))

    # stampo le dir del sottoalbero (se True) o solo quelle della directory path (se False)
    # print('\n'.join(get_files_and_directories(path, sottoalbero=False)))
