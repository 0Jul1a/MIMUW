# Python_WikiScraper
Python Course Project -  Faculty of Mathematics, Informatics, and Mechanics, University of Warsaw

Źródło danych z których pobierane i analizowane są dane w tym projekcie: https://bulbapedia.bulbagarden.net/wiki

# Podstawowe funkcjonalności programu (sposoby uruchomienia):
* python wiki_scraper.py --summary "szukana fraza"
* python wiki_scraper.py --table "szukana fraza"
--number n [--first-row-is-header]
* python wiki_scraper.py --count-words "szukana fraza"
* python wiki_scraper.py --analyze-relative-word-frequency --mode "tryb"
--count n
[--chart "ścieżka/do/pliku.png"]
* python wiki_scraper.py --auto-count-words "początkowa szukana fraza" --depth n --wait t

Aby odpalić testy znajdujące się w src/tests należy użyc komency cd src/tests a następnie, aby odpalić testy jednostkowe: pytest testy_jednostkowe.py, aby odpalić test integracyjny "wiki_scraper_integration_test.py": python wiki_scraper_integration_test.py

Aby dodać wszystkie zależności należy wpisać pip freeze > requirements.txt
