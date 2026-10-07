from collections import deque
from urllib.parse import urljoin
import time

import count_words as c

WIKI_PREFIX = "/wiki/"


def get_links(soup):
    links = set()  # Aby nie było przypadkiem sytuacji, w której link jest dwa razy dodany
    for a in soup.find_all('a', href=True):
        href = a['href']

        if "#" in href:  # Odwołanie do tej samej strony, więc nie bierzemy pod uwagę
            continue
        if href.startswith(WIKI_PREFIX) and ":" not in href:
            links.add(href)

    return links


def bfs(base_url, title, max_depth, wait):
    import scraper as s
    new_title = title.replace(" ", "_")
    visited_urls = set()
    queue = deque([(new_title, 0)])

    while queue:
        url, depth = queue.popleft()

        if url in visited_urls:
            continue
        visited_urls.add(url)

        scraper = s.Scraper(base_url, title)
        soup = scraper.scrape()
        full_url = urljoin(base_url, url)

        print(f"I'm processing the phrase: {full_url}")
        c.count(soup.get_text())

        if depth < max_depth:
            links = get_links(soup)
            for link in links:
                if link not in visited_urls:
                    queue.append((link, depth + 1))

        time.sleep(wait)
