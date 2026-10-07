import html

import requests
from bs4 import BeautifulSoup


class Scraper:
    def __init__(self, source, title, use_local_html_file_instead=False):
        self.source = source
        self.title = title
        self.use_local_html_file_instead = use_local_html_file_instead

    def scrape(self):
        # Jeśli chcemy z pliku pobrać kod
        if self.use_local_html_file_instead:
            with self.source.open("r") as f:
                raw_html = f.read()

            html_content = html.unescape(raw_html)  # zmieniamy &lt;div&gt; → <div>
            soup = BeautifulSoup(html_content, "html.parser")

            content = soup.select_one(
                "div.mw-content-ltr.mw-parser-output")  # Bierzemy tylko treść artykułu bz menu, stopek itp.
            if content is None:
                print(f"Error! content not found in file {self.source}")
                return None

            return content

        new_title = self.title.replace(" ", "_")
        url = self.source + new_title

        response = requests.get(url)
        if response.status_code != 200:
            print(f"Error! response status = {response.status_code}")
            return None

        soup = BeautifulSoup(response.text, "html.parser")

        content = soup.select_one("div#mw-content-text div.mw-parser-output")
        if content is None:
            print(f"Error! content not found on {url}")
            return None

        return content

    def scrape_nth_table(self, data, n):
        tables = data.find_all("table")
        table = tables[n - 1]  # Ponieważ tabele są pod inseksami 0,...,n-1
        rows = table.find_all("tr")
        grid = []
        spans = {}

        for i, row in enumerate(rows):
            r = []  # Aktualny wiersz tabeli
            col_index = 0

            for cell in row.find_all(["td", "th"]):

                while (i, col_index) in spans:
                    r.append(spans[(i, col_index)])
                    col_index += 1

                text = cell.get_text(strip=True)
                row_span = int(cell.get("rowspan", 1))  # Ile wierszy w dół ma taką samą wartość
                col_span = int(cell.get("colspan", 1))  # Ile kolumn w bok ma taką samą wartość

                for _ in range(col_span):
                    r.append(text)
                    if row_span > 1:
                        for rs in range(row_span):
                            spans[(i + rs, col_index)] = text
                    col_index += 1

            grid.append(r)

        return grid
