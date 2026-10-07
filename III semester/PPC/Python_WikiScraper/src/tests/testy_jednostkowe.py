import sys
from pathlib import Path

from bs4 import BeautifulSoup

ROOT = Path(__file__).parent.parent
SRC = ROOT / "tasks"
sys.path.insert(0, str(SRC))

from summary import summarize as s
from count_words import tokenize as t
from count_words import count_all_words as c
from auto_count_words import get_links as g


def test_tokenize():
    text = "Team Rocket is a villainous team in pursuit of evil and the exploitation of Pokémon."
    new_text = t(text)
    expected_ans = ['team', 'rocket', 'is', 'a', 'villainous', 'team', 'in', 'pursuit', 'of', 'evil', 'and', 'the',
                    'exploitation', 'of', 'pokémon']
    assert new_text == expected_ans


# Testujemy czy funkcja count_all_words z "count_words" poprawnie zwraca słownik z policzonymi słowami
def test_count_words():
    text = "Team Rocket is a villainous team in pursuit of evil and the exploitation of Pokémon."
    new_text = t(text)
    expected_ans = {
        "team": 2,
        "rocket": 1,
        "is": 1,
        "a": 1,
        "villainous": 1,
        "in": 1,
        "pursuit": 1,
        "of": 2,
        "evil": 1,
        "and": 1,
        "the": 1,
        "exploitation": 1,
        "pokémon": 1,
    }
    assert c(new_text) == expected_ans


# Testujemy czy poprawnie wybiera linki
def test_get_links():
    html = """
    <a href="/wiki/Pikachu">P</a>
    <a href="/wiki/File:Image.jpg">I</a>
    <a href="/wiki/Charmander#History">C</a>
    """
    soup = BeautifulSoup(html, "html.parser")
    links = g(soup)
    assert links == {'/wiki/Pikachu'}


def test_summarize():
    html = """<p><b>Team Rocket</b> (Japanese: <b><span lang="ja" style="margin: 0px;">ロケット<ruby lang="ja">団<rp>(</rp><rt>だん</rt><rp>)</rp></ruby></span></b> <i>Rocket-dan</i>, literally <i>Rocket Gang</i>) is a <a href="/wiki/Villainous_team" title="Villainous team">villainous team</a> in pursuit of evil and the exploitation of <a href="/wiki/Pok%C3%A9mon_(species)" title="Pokémon (species)">Pokémon</a>. The organization is based in the <a href="/wiki/Kanto" title="Kanto">Kanto</a> and <a href="/wiki/Johto" title="Johto">Johto</a> <a href="/wiki/Region" title="Region">regions</a>, with a small outpost in the <a href="/wiki/Sevii_Islands" title="Sevii Islands">Sevii Islands</a>.
</p><p>While its main focus is stealing or capturing rare and strong Pokémon, and subsequently selling them, it also funds and conducts cruel experimental research on Pokémon. At times, it has even <a href="/wiki/Marowak_(ghost)" title="Marowak (ghost)">killed</a> Pokémon. Their oath, as posted on the wall of <a href="/wiki/Team_Rocket_HQ" title="Team Rocket HQ">one</a> of their many bases, is "Steal Pokémon for profit. Exploit Pokémon for profit. All Pokémon exist for the glory of Team Rocket." Their ultimate goal is to take over the world using Pokémon.
</p><p>An alternate version of Team Rocket named <a href="/wiki/Team_Rainbow_Rocket" title="Team Rainbow Rocket">Team Rainbow Rocket</a> appeared in <a href="/wiki/Alola" title="Alola">Alola</a> from a <a href="/wiki/Pok%C3%A9mon_multiverse" title="Pokémon multiverse">parallel universe</a> through an <a href="/wiki/Ultra_Wormhole" title="Ultra Wormhole">Ultra Wormhole</a>.
</p>
<div aria-labelledby="mw-toc-heading" class="toc" id="toc" role="navigation"><input class="toctogglecheckbox" id="toctogglecheckbox" role="button" style="display:none" type="checkbox"><div class="toctitle" dir="ltr" lang="en"><h2 id="mw-toc-heading">Contents</h2><span class="toctogglespan"><label class="toctogglelabel" for="toctogglecheckbox"></label></span></div>"""
    soup = BeautifulSoup(html, "html.parser")
    summary = s(soup)
    expected_ans = "Team Rocket (Japanese: ロケット団 Rocket-dan, literally Rocket Gang) is a villainous team in pursuit of evil and the exploitation of Pokémon. The organization is based in the Kanto and Johto regions, with a small outpost in the Sevii Islands."
    assert summary == expected_ans
