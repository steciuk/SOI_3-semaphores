# SOI_3-semaphores
## System docelowy:  
System docelowy składa się z 3 programów:
1. administrator (admwriter.c) – pełniący również rolę serwera; implementuje listę
przechowywującą wiadomości. Wielkość listy ustalamy w pliku konfiguracyjnym (shmem.h) w
polu (MAX_MSG). W procesie administratora uruchamiany jest niezależny wątek (listener)
nasłuchujący czy w pamięci współdzielonej nie pojawiły się nowe wiadomości. W listenerze
obsługiwane jest zaalokowanie pamięci wspódzielonej pomiędzy procesami oraz stworzenie
semaforów międzyprocesoestwych (semptr, semptr_full, semptr_empty). Listener,
wiadomości od użytkowników wpisuje na końcu bufora wiadomości a od vipów – na
początku. Natomiast funkcja main wita użytkownika poprzez menu pozwalające na:
  - „wysłanie” wiadomości (usuwa pierwszą w buforze wiadomość i wypisuje ją na
wyjście standardowe)
  - wyświetlenie całego bufora wiadomości
  - usunięcie zawartości całego bufora
Współdzielony dostęp do globalnego bufora wiadomości (messages) ograniczony jest za
pomocą semaforów międzywątkowych (list_lock, list_empty, list_full, tr_end).
2. user (usrwriter.c) – dołącza współdzielony obszar pamięci i umożliwia użytkownikowi
wysyłanie wiadomości (wraz z identyfikatorem typu procesu) do pamięci współdzielonej
3. vip (vipwritter.c) – identycznie jak user, tylko wiadomości z innym identyfikatorem

## System testowy:
System testowy skonstruowany został tak żeby przez manipulacje danymi wejściowymi móc
doprowadzić do sytuacji niebezpiecznych i wyjątkowych.
1. administrator (admtest.c) – jak program docelowy z dodatkową funkcjonalnością
pozwalającą na ustawienie interwałów z jakimi „wysyła” wiadomości z bufora (opróżnia
pojedynczo bufor) oraz ilości zczytań przed zakończeniem działania.
2. user (usertest.c) – jak program docelowy z dodatkową funkcjonalnością pozwalającą na
ustawienie interwałów z jakimi wysyłane są do pamięci współdzielonej automatycznie
generowane wiadomości, zawierające dodatkowo numer wiadomości oraz (możliwy od
ustawienia) identyfikator konkretnego procesu
3. vip (viptest.c) – identycznie jak user, tylko z innym identyfikatorem

## Testy:
Następujące potencjalnie niebezpieczne sytuacje testuję odpowiednio dobierając dane testowe,
doprowadzające do konkretnych sytuacji:
1. Lista jest pusta, a:  
  a) administrator próbuje z wysłać z niej elementy  
  b) administrator próbuje wyświetlić elementy  
  c) administrator próbuje usunąć z niej elementy  
  
Testy nieautomatyczne do 1.a, 1.b, 1.c przeprowadzone na programach docelowych, pokazanie działania docelowego programu:
![Obraz1](https://user-images.githubusercontent.com/48189079/110817964-1b014600-828d-11eb-9e3c-f6f8d7efc677.png)
![Obraz2](https://user-images.githubusercontent.com/48189079/110818092-3e2bf580-828d-11eb-99a6-9142f94c2d2d.png)  
Test automatyczny przedstawiający sposób działania systemu w wariancie niedosycenia wiadomościami. Sprawdza sytuację 1.a.

2. Lista jest pełna a:  
  a) vip próbuje wysłać wiadomość  
  b) user próbuje wysłać wiadomość  
![Obraz3](https://user-images.githubusercontent.com/48189079/110818372-7af7ec80-828d-11eb-8a33-ed26059103a3.png)
![Obraz4](https://user-images.githubusercontent.com/48189079/110818380-7df2dd00-828d-11eb-9695-f8e3ce9a1eb7.png)

Powyższe testy automatyczne pokazują, że i vip i user oczekują na zwolnienie się miejsca w liście w sytuacji przesycenia systemu wiadomościami oraz reagują tuż po jego pojawieniu się.

