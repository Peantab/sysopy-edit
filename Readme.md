# Środowisko do kooperatywnej edycji tekstów (English below)
## SysOpy Edit
Paweł Taborowski

## Co to?

Edytor tekstu, który łączy się z serwerem i wymienia z nim na bieżąco informacje dotyczące edytowanego dokumentu. Klientów oczywiście może być wielu, komunikują się z serwerem z pomocą gniazd TCP.

Został napisany na jako projekt na przedmiot Systemy Operacyjne.

## Technologia
### Język programowania – C++
* Bo środowisko graficzne
* Wygoda używania edytora tekstu

### Środowisko graficzne – Qt
* Jedno z dwóch do wyboru (treść zadania)
* KDE, Canonical (Ubuntu), Nokia, EA (Origin), AMD, Blizzard...

### Komunikacja – Sockety
* Do sieci tylko sockety
* Połączeniowe – nic nie zginie

## Instalacja
* Oczywiście wymaga Qt (używałem wersji 5.8; qmake z repo Ubuntu nie wystarczy)
* To dwa "niezależne" programy o podobnych wymaganiach
* `qmake` następnie `make`
* Ale lepiej przez Qt Creator...
* Czynności powtórzyć dla drugiego programu

## Konfiguracja
* Plik w katalogu edytora (klienta) `configSOE`
* Ma w pierwszej linii adres IP (v4), w drugiej numer portu.
  ```
  127.0.0.1
  6000
  ```

## Uruchomienie
* Serwer przyjmuje numer portu: `./sysOpyServer 6000`
* Można uruchamiać wielu klientów `./sysOpyEditor` (można też wyklikać w menadżerze plików).
* Klientów zamykamy krzyżykiem, wówczas serwer wyłącza się automatycznie, dokonując zapisu pliku na dysk.

## Jak to działa?

* Serwer
  * Jest prostą konsolową aplikacją
  * Odczytuje początkową wersję dokumentu z dysku
  * Wysyła klientom aktualną wersję dokumentu
  * Odbiera od klientów zmodyfikowane wersje
* Klient
  * Aplikacja Qt Widgets (GUI)
  * Łączy się z serwerem i pobiera pierwotną wersję dokumentu
  * Wysyła na serwer dokument zmodyfikowany przez użytkownika
  * Odbiera aktualizacje z serwera

# [English]
Project created for the Operating Systems course.

You can build both client and server (separately) with `qmake` and then `make`, then run the server with `./sysOpyServer 6000` (where 6000 is an examplary port number) and the client with `./sysOpyEditor`.

You can modify client's listening IP address and port number by editing `configSOE` in editor's directory. It's default content is:
```
127.0.0.1
6000
```
