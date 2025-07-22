'''Scrivere un programma python o uno script bash che esaminando i dati in /proc/*/status fornisca in
output una tabella che indichi per ogni processo di proprietà dell'utente, il nome dell'eseguibile e
l'attuale occupazione di memoria (campo vmSize).'''


import os

def get_process_info():
    uid = os.getuid()
    results = []

    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            with open(f"/proc/{pid}/status") as f:
                name = ""
                mem = ""
                is_mine = False
                for line in f:
                    if line.startswith("Name:"):
                        name = line.split()[1]
                    elif line.startswith("Uid:"):
                        uid_fields = line.split()
                        real_uid = int(uid_fields[1])
                        if real_uid == uid:
                            is_mine = True
                    elif line.startswith("VmSize:"):
                        mem = " ".join(line.split()[1:])
                if is_mine:
                    results.append((pid, name, mem if mem else "0 kB"))
        except FileNotFoundError:
            continue
        except PermissionError:
            continue

    return results

def print_table(processes):
    print(f"{'PID':<8} {'Name':<25} {'VmSize':<15}")
    print("-" * 50)
    for pid, name, mem in processes:
        print(f"{pid:<8} {name:<25} {mem:>15}")

if __name__ == "__main__":
    processes = get_process_info()
    print_table(processes)
