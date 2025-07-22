1. **capitalize()**: Restituisce una copia della stringa con il primo carattere maiuscolo.
   ```python
   s = "hello"
   print(s.capitalize())  # Output: "Hello"
   ```

2. **casefold()**: Restituisce una versione della stringa in minuscolo adatta per confronti di casi non sensibili.
   ```python
   s = "HELLO"
   print(s.casefold())  # Output: "hello"
   ```

3. **center(width, fillchar=' ')**: Centra la stringa in un campo di larghezza specificata.
   ```python
   s = "hello"
   print(s.center(10, '-'))  # Output: "--hello---"
   ```

4. **count(sub, start=0, end=None)**: Conta il numero di occorrenze di una sottostringa.
   ```python
   s = "hello world"
   print(s.count('o'))  # Output: 2
   ```

5. **encode(encoding='utf-8', errors='strict')**: Restituisce una versione codificata della stringa.
   ```python
   s = "hello"
   print(s.encode())  # Output: b'hello'
   ```

6. **endswith(suffix, start=0, end=None)**: Verifica se la stringa termina con il suffisso specificato.
   ```python
   s = "hello"
   print(s.endswith('o'))  # Output: True
   ```

7. **expandtabs(tabsize=8)**: Sostituisce i caratteri di tabulazione con spazi.
   ```python
   s = "hello\tworld"
   print(s.expandtabs(4))  # Output: "hello   world"
   ```

8. **find(sub, start=0, end=None)**: Trova la prima occorrenza di una sottostringa.
   ```python
   s = "hello"
   print(s.find('l'))  # Output: 2
   ```

9. **format(*args, **kwargs)**: Format string using placeholders.
   ```python
   s = "Hello, {}"
   print(s.format("world"))  # Output: "Hello, world"
   ```

10. **format_map(mapping)**: Simile a format(), ma utilizza un dizionario.
    ```python
    s = "Hello, {name}"
    print(s.format_map({'name': 'world'}))  # Output: "Hello, world"
    ```

11. **index(sub, start=0, end=None)**: Trova la prima occorrenza di una sottostringa e restituisce l'indice.
    ```python
    s = "hello"
    print(s.index('l'))  # Output: 2
    ```

12. **isalnum()**: Verifica se tutti i caratteri nella stringa sono alfanumerici.
    ```python
    s = "hello123"
    print(s.isalnum())  # Output: True
    ```

13. **isalpha()**: Verifica se tutti i caratteri nella stringa sono alfabetici.
    ```python
    s = "hello"
    print(s.isalpha())  # Output: True
    ```

14. **isascii()**: Verifica se tutti i caratteri nella stringa sono ASCII.
    ```python
    s = "hello"
    print(s.isascii())  # Output: True
    ```

15. **isdigit()**: Verifica se tutti i caratteri nella stringa sono cifre.
    ```python
    s = "123"
    print(s.isdigit())  # Output: True
    ```

16. **isidentifier()**: Verifica se la stringa è un identificatore valido.
    ```python
    s = "hello"
    print(s.isidentifier())  # Output: True
    ```

17. **islower()**: Verifica se tutti i caratteri nella stringa sono minuscoli.
    ```python
    s = "hello"
    print(s.islower())  # Output: True
    ```

18. **isnumeric()**: Verifica se tutti i caratteri nella stringa sono numerici.
    ```python
    s = "123"
    print(s.isnumeric())  # Output: True
    ```

19. **isprintable()**: Verifica se tutti i caratteri nella stringa sono stampabili.
    ```python
    s = "hello"
    print(s.isprintable())  # Output: True
    ```

20. **isspace()**: Verifica se tutti i caratteri nella stringa sono spazi bianchi.
    ```python
    s = "   "
    print(s.isspace())  # Output: True
    ```

21. **istitle()**: Verifica se la stringa è in formato "title case".
    ```python
    s = "Hello World"
    print(s.istitle())  # Output: True
    ```

22. **isupper()**: Verifica se tutti i caratteri nella stringa sono maiuscoli.
    ```python
    s = "HELLO"
    print(s.isupper())  # Output: True
    ```

23. **join(iterable)**: Restituisce una stringa che è la concatenazione delle stringhe nell'iterabile.
    ```python
    s = "-"
    print(s.join(["hello", "world"]))  # Output: "hello-world"
    ```

24. **ljust(width, fillchar=' ')**: Allinea la stringa a sinistra in un campo di larghezza specificata.
    ```python
    s = "hello"
    print(s.ljust(10, '-'))  # Output: "hello-----"
    ```

25. **lower()**: Restituisce una copia della stringa in minuscolo.
    ```python
    s = "HELLO"
    print(s.lower())  # Output: "hello"
    ```

26. **lstrip(chars=None)**: Rimuove i caratteri iniziali specificati (gli spazi bianchi per default).
    ```python
    s = "  hello"
    print(s.lstrip())  # Output: "hello"
    ```

27. **partition(sep)**: Divide la stringa in un tuple in corrispondenza del primo separatore trovato.
    ```python
    s = "hello world"
    print(s.partition(" "))  # Output: ('hello', ' ', 'world')
    ```

28. **replace(old, new, count=-1)**: Sostituisce le occorrenze della sottostringa old con new.
    ```python
    s = "hello world"
    print(s.replace("world", "Python"))  # Output: "hello Python"
    ```

29. **rfind(sub, start=0, end=None)**: Trova l'ultima occorrenza di una sottostringa.
    ```python
    s = "hello"
    print(s.rfind('l'))  # Output: 3
    ```

30. **rstrip(chars=None)**: Rimuove i caratteri finali specificati (gli spazi bianchi per default).
    ```python
    s = "hello   "
    print(s.rstrip())  # Output: "hello"
    ```

Questi metodi coprono una vasta gamma di operazioni comuni sulle stringhe in Python, inclusa la formattazione, la ricerca, la sostituzione e la verifica delle proprietà delle stringhe.
