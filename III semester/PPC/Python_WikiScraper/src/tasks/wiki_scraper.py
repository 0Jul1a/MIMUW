import argparse

import scraper
import summary
import table
import count_words
import analyze_relative_word_frequency
import auto_count_words

URL_TO_SCRAPE = "https://bulbapedia.bulbagarden.net/wiki/"


def summary_arg(args):
    scraper_ = scraper.Scraper(URL_TO_SCRAPE, args.summary)
    scraped_data = scraper_.scrape()

    if scraped_data is None:
        print(f"Error! No data for {args.summary}")
        return

    print(summary.summarize(scraped_data) + "\n")


def table_arg(args):
    scraper_ = scraper.Scraper(URL_TO_SCRAPE, args.table)
    scraped_data = scraper_.scrape()

    if scraped_data is None:
        print(f"Error! No data for {args.table}")
        return

    table_data = scraper_.scrape_nth_table(scraped_data, args.number)
    print(table.get_nth_table(table_data, args.first_row_is_header, args.table))


def count_arg(args):
    scraper_ = scraper.Scraper(URL_TO_SCRAPE, args.count_words)
    scraped_data = scraper_.scrape()

    if scraped_data is None:
        print(f"Error! No data for {args.count_words}")
        return

    count_words.count(scraped_data)
    print("Data generated and saved in word-counts.json")


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument("--summary", help="Use: --summary 'szukana fraza'")

    parser.add_argument("--table", help="Use: --table 'szukana fraza' --number n")
    parser.add_argument("--number", type=int)
    parser.add_argument("--first-row-is-header", action="store_true")

    parser.add_argument("--count-words", help="Use: --count-words 'szukana fraza'")

    parser.add_argument("--analyze-relative-word-frequency", action="store_true",
                        help="Use: --analyze-relative-words --mode 'tryb' --count n")
    parser.add_argument("--mode")
    parser.add_argument("--count", type=int)
    parser.add_argument("--chart")

    parser.add_argument("--auto-count-words",
                        help="Use: --auto-count-words 'początkowo szukana fraza' --depth n --wait t")
    parser.add_argument("--depth", type=int)
    parser.add_argument("--wait", type=int)

    parsed_args = parser.parse_args()
    return parsed_args


def main():
    args = parse_args()
    if args.summary:
        summary_arg(args)
    elif args.table:
        table_arg(args)
    elif args.count_words:
        count_arg(args)
    elif args.analyze_relative_word_frequency:
        lang = "en"
        analyze_relative_word_frequency.analyze(args.mode, args.count, args.chart, lang)
    elif args.auto_count_words:
        auto_count_words.bfs(URL_TO_SCRAPE, args.auto_count_words, args.depth, args.wait)
    print(
        f"Wyjście programu na licencji BY-NC-SA stworzone na podstawie (pod)artykułu/ów dostępnego/ych na stronie https://bulbapedia.bulbagarden.net/wiki.")


if __name__ == "__main__":
    main()
