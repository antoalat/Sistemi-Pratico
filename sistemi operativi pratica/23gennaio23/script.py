'''
Scrivere uno script bash o python difidr che date due directory ne crei una terza e una quarta
Le nuove directory devono contenere solamente i file aventi lo stesso nome presenti nella prima e nella seconda
albero. Ogni file della terza directory deve contenere una copia del file nella versione della prima directory,
mentre nella quarta directory la versione della seconda.
es.
* se la direcotry a contiene i file alpha, beta, gamma, delta e la directory b i file beta, delta, epsilon, zeta
il comando "difdir a b newa newb" crea le directory newa e newb ed entrambe le directory devono
contenere solo beta e delta (i nomi in comune). newa/beta deve essere una copia di a/beta mentre
newb/beta una copia di b/beta. In modo simile per a/delta b/delta newa/delta e newb/delta.
'''

import os
import sys
import shutil


def find_file_in_dir (dir, file):
    for entry in os.listdir(dir):
        full_path = os.path.join(dir,entry)
        if os.path.isfile(full_path) and file == entry:
            return True
    return False

if __name__ == "__main__":
    if(len(sys.argv)!= 5):
        raise Exception(f"Uso: {sys.argv[0]} <dira> <dirb> <newa> <newb>")
    
    dira = sys.argv[1]
    dirb = sys.argv[2]
    newdira = sys.argv[3]
    newdirb = sys.argv[4]
    
    if((not os.path.isdir(dira)) or (not os.path.isdir(dirb))):
        raise Exception("Directory non valide")
    
    try:
        os.makedirs(newdira)
        os.makedirs(newdirb)
    except OSError as e:
        print(f"Errore {e} durante la creazione delle cartelle")
    
    for entry in os.listdir(dira):
        full_path = os.path.join(dira, entry)
        if os.path.isfile(full_path) and find_file_in_dir(dirb,entry):
            try:
                shutil.copy(full_path, newdira)
            except OSError as e:
                print(f"Errore copia file {e}")
    
    
    for entry in os.listdir(dirb):
        full_path = os.path.join(dirb, entry)
        if os.path.isfile(full_path) and find_file_in_dir(dira,entry):
            try:
                shutil.copy(full_path, newdirb)
            except OSError as e:
                print(f"Errore copia file {e}")        

        
        