import json
import re
from collections import Counter  # Counter: słownik, ktory zlicza wystąpienia elementów
from pathlib import Path

import regex  # dowolna litere Unicode

BASE_FILE = "./word-counts.json"

def tokenize(text):
    if not text:
        print("No text!")
        return None

    if not isinstance(text, str):
        text = text.get_text()

    text = text.lower()  # Traktujemy wszystko jako małe litery
    text = re.sub(r'https?://\S+', '', text)  # Usuwamy wszelkie URL-e
    text = regex.sub(r"[^\p{L}0-9\s]", "",
                     text)  # Usuwamy wszystkie niepotrzebne elemety oprócz liter (również polskihc), liczb i znaków białych
    return text.split()  # Dzielimy tekst


def count_all_words(data):
    return Counter(data)


def prev_count(file):
    if file.exists():
        with open(file, "r") as f:
            return Counter(json.load(f))
    return Counter()  # Pusty licznik słów


def save_count(counter, file):
    with open(file, "w") as f:
        json.dump(counter, f, ensure_ascii=False,
                  indent=2)  # ensure_ascii = False, aby dobrze zapisało polskie znaki, indent=2: odpowiedni format w jsonie


def count(data, different_file=BASE_FILE):
    file_path = Path(different_file)

    text = tokenize(data)

    if text is None:
        return

    current_count = count_all_words(text)
    prev_count_ = prev_count(file_path)
    prev_count_.update(current_count)
    save_count(prev_count_, file_path)
