'''Scrivere uno script in grado si cercare all'interno di un sottoalbero del file system il file modificato più
di recente e quello la cui ultima modifca è avvenuta più anticamente.
'''


import os
import sys
from datetime import datetime


if __name__ == "__main__":
    if (len(sys.argv) != 2):
        raise Exception(f"Uso: {sys.argv[0]} <directory>")
    dir = sys.argv[1]
    d =  []
    for root, dirs, files in os.walk(dir):
        for file in files:
            full_path = os.path.join(root,file)
            stats = os.stat(full_path)
            timestamp = stats.st_mtime
            d.append((full_path, timestamp))
            
    sorted_list = sorted(d, key=lambda x: x[1], reverse=True)
    print(f"FIle modificato più di recente: {sorted_list[0][0]} {datetime.fromtimestamp(sorted_list[0][1])}")
    print(f"FIle modificato meno di recente: {sorted_list[-1][0]} {datetime.fromtimestamp(sorted_list[-1][1])}")