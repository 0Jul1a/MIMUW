import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent
SRC = ROOT / "tasks"
sys.path.insert(0, str(SRC))

import scraper as s
from summary import summarize as sc


def test_summarize():
    # Wyciągamy ścieżkę katalogu w którym jest ten test
    base_dir = Path(__file__).resolve().parent
    html_file = base_dir / "Team_Rocket.html"

    scraper = s.Scraper(html_file, " ", True)
    scraped_data = scraper.scrape()
    try:
        summary = sc(scraped_data)
        assert summary is not None
        assert summary.startswith("Team Rocket"), f"Wrong summary {summary} should start with Team Rocket"
        assert summary.endswith(
            "outpost in the Sevii Islands."), f"Wrong summary {summary} should end with outpost in the Sevii Islands"
        print("Success!")

    except Exception as e:
        print(f"Error: {e}")
        exit(1)


if __name__ == "__main__":
    test_summarize()
