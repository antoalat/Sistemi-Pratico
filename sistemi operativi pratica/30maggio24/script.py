  '''
  Esercizio 3: Python o bash: 10 punti
Scrivere un programma o uno script lscmd che consenta ad un utente di poter avere l'elenco di tutti i
pid dei suoi processi in esecuzione raggruppati per pathname del programma eseguito..
Es:
 $ lscmd
 /usr/bin/bash 2021 2044
 /usr/bin/xterm 2010
,,,
  '''
  
  
  
    import os
    import sys
    from collections import defaultdict


    def get_uid_of_process(pid):
        try:
            with open(f"/proc/{pid}/status") as f:
                for line in f:
                    if line.startswith("Uid:"):
                        return int(line.split()[1])  # UID reale
        except:
            return None

    if __name__ == "__main__":
        if(len(sys.argv) != 1):
            raise Exception(f"Uso: {sys.argv[0]}")
        d = defaultdict(list)
        my_id = os.getuid()
        for file in os.listdir("/proc"):
            if get_uid_of_process(file) == my_id:
                full_path = os.path.join("/proc", file,"exe")
                if(os.path.islink(full_path)):
                    real_link = os.readlink(full_path)
                    d[real_link].append(file)
                    
                
        
        for link, pids in d.items():
            print(f"link eseguibile: {link}")
            for pid in pids:
                print(pid)
        
        





import psutil


def lscmd():
    d = {}
    pids = psutil.pids()
    for pid in pids:
        path = ""
        try:
            path = psutil.Process(pid).exe()
        except Exception as e:
            pass

        if path not in d.keys():
            d[path] = [pid]
        else:
            d[path].append(pid)

    for k, v in d.items():
        print(k, v)


if __name__ == "__main__":
    lscmd()