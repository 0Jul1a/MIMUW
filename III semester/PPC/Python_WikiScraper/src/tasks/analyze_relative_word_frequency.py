from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from wordfreq import top_n_list, word_frequency

import count_words as c

FILE = Path("word-counts.json")


def analyze(mode, n, chart=None, lang="en"):
    if not FILE.exists():
        print("File not found, probably not created yet")
        return

    data = c.prev_count(FILE)  # Pobieramy naszą częstotliwość słów z pliku "word-counts.json"
    table = []

    if mode == "article":
        top_n_common = data.most_common(n)

        for word, freq in top_n_common:
            lang_freq = word_frequency(word, lang)
            if lang_freq == 0:
                lang_freq = None

            table.append(
                {
                    "Word": word,
                    "Frequency in the article": freq,
                    "Frequency in wiki language": lang_freq,
                }
            )

    elif mode == "language":
        top_n_common = top_n_list(lang, n)
        for word in top_n_common:
            table.append({
                "Word": word,
                "Frequency in the article": data.get(word),
                "Frequency in wiki language": word_frequency(word, lang),
            })
    else:
        print("Unknown mode, use article or language")
        return

    df = pd.DataFrame(table)

    # Normalizujemy częstotliwości poprzez podzielenie ich przez częstotliwość najczęstszego słowa
    df["Frequency in the article"] /= df["Frequency in the article"].max()
    df["Frequency in wiki language"] /= df["Frequency in wiki language"].max()

    # Sortujemy
    if mode == "article":
        df = df.sort_values("Frequency in the article", ascending=False)
    elif mode == "language":
        df = df.sort_values("Frequency in wiki language", ascending=False)

    print(df[["Word", "Frequency in the article", "Frequency in wiki language"]].to_string(index=False))

    if chart:  # Rysujemy wykres i zapisujemy do ścieżki "chart"
        plt.figure(figsize=(10, 6))

        x = np.arange(len(df))
        width = 0.35

        plt.title("Frequency of some words on Bulbapedia")
        plt.bar(x - width / 2, df["Frequency in the article"], width, label="Bulbapedia", color="blue")
        plt.bar(x + width / 2, df["Frequency in wiki language"], width, label="English", color="red")

        plt.xticks(x, df["Word"])
        plt.ylabel("Frequency")
        plt.legend()
        plt.tight_layout()
        plt.savefig(chart)
        plt.close()
