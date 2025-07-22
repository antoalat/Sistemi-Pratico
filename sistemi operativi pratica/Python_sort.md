1. **Utilizzare il metodo `sort()` per liste**:
   - Il metodo `sort()` ordina la lista in-place, modificando la lista originale.
   ```python
   lista = [3, 1, 4, 1, 5, 9, 2, 6, 5]
   lista.sort()
   print(lista)  # Output: [1, 1, 2, 3, 4, 5, 5, 6, 9]
   ```

2. **Utilizzare la funzione `sorted()` per liste**:
   - La funzione `sorted()` restituisce una nuova lista ordinata, lasciando inalterata la lista originale.
   ```python
   lista = [3, 1, 4, 1, 5, 9, 2, 6, 5]
   ordinata = sorted(lista)
   print(ordinata)  # Output: [1, 1, 2, 3, 4, 5, 5, 6, 9]
   print(lista)  # Output: [3, 1, 4, 1, 5, 9, 2, 6, 5]
   ```

3. **Ordinare una lista di dizionari per una chiave specifica**:
   - Utilizzare `sorted()` con un parametro `key`.
   ```python
   lista_dizionari = [{'nome': 'Alice', 'età': 25}, {'nome': 'Bob', 'età': 30}, {'nome': 'Charlie', 'età': 20}]
   ordinata = sorted(lista_dizionari, key=lambda x: x['età'])
   print(ordinata)  # Output: [{'nome': 'Charlie', 'età': 20}, {'nome': 'Alice', 'età': 25}, {'nome': 'Bob', 'età': 30}]
   ```

4. **Ordinare una lista di tuple**:
   - Utilizzare `sorted()` con un parametro `key`.
   ```python
   lista_tuples = [(1, 'z'), (2, 'x'), (1, 'y'), (3, 'a')]
   ordinata = sorted(lista_tuples, key=lambda x: (x[0], x[1]))
   print(ordinata)  # Output: [(1, 'y'), (1, 'z'), (2, 'x'), (3, 'a')]
   ```

5. **Ordinare un dizionario per chiavi**:
   - Convertire il dizionario in una lista di tuple, ordinare e poi convertire di nuovo in un dizionario.
   ```python
   dizionario = {'b': 1, 'a': 2, 'c': 3}
   ordinato = dict(sorted(dizionario.items()))
   print(ordinato)  # Output: {'a': 2, 'b': 1, 'c': 3}
   ```

6. **Ordinare un dizionario per valori**:
   - Utilizzare `sorted()` su `dizionario.items()` con un parametro `key`.
   ```python
   dizionario = {'b': 1, 'a': 3, 'c': 2}
   ordinato = dict(sorted(dizionario.items(), key=lambda item: item[1]))
   print(ordinato)  # Output: {'b': 1, 'c': 2, 'a': 3}
   ```

7. **Ordinare una lista in ordine decrescente**:
   - Utilizzare `sort()` o `sorted()` con il parametro `reverse=True`.
   ```python
   lista = [3, 1, 4, 1, 5, 9, 2, 6, 5]
   lista.sort(reverse=True)
   print(lista)  # Output: [9, 6, 5, 5, 4, 3, 2, 1, 1]

   ordinata = sorted(lista, reverse=True)
   print(ordinata)  # Output: [9, 6, 5, 5, 4, 3, 2, 1, 1]
   ```

8. **Utilizzare `numpy` per ordinare array**:
   - Usare la funzione `numpy.sort()`.
   ```python
   import numpy as np
   array = np.array([3, 1, 4, 1, 5, 9, 2, 6, 5])
   ordinato = np.sort(array)
   print(ordinato)  # Output: [1 1 2 3 4 5 5 6 9]
   ```

9. **Ordinare una stringa**:
   - Utilizzare `sorted()` su una stringa per ottenere una lista di caratteri ordinati, poi usare `''.join()` per ottenere una stringa.
   ```python
   stringa = "python"
   ordinata = ''.join(sorted(stringa))
   print(ordinata)  # Output: 'hnopty'
   ```

10. **Ordinare una lista di stringhe ignorando maiuscole/minuscole**:
    - Utilizzare `sorted()` con un parametro `key` che converte ogni stringa in minuscolo.
    ```python
    lista_stringhe = ["banana", "Apple", "cherry"]
    ordinata = sorted(lista_stringhe, key=lambda x: x.lower())
    print(ordinata)  # Output: ['Apple', 'banana', 'cherry']
    ```

Questi metodi coprono una varietà di scenari di ordinamento comuni in Python, inclusi liste, dizionari, stringhe e array numerici.