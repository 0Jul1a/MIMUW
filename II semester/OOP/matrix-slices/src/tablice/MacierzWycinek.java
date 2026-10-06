package tablice;

public class MacierzWycinek extends Macierz {

    private final Macierz oryginalnaMacierz;
    private final int wierszPoczatek;
    private final int wierszKoniec;
    private final int kolumnaPoczatek;
    private final int kolumnaKoniec;

    public MacierzWycinek(Macierz _oryginalnaMacierz, int _wierszPoczatek, int _wierszKoniec, int _kolumnaPoczatek, int _kolumnaKoniec) {
        super(przytnijMacierz(_oryginalnaMacierz, _wierszPoczatek, _wierszKoniec + 1, _kolumnaPoczatek, _kolumnaKoniec + 1));

        if (_wierszPoczatek < 0 || _wierszKoniec > _oryginalnaMacierz.ksztalt()[0] || _wierszPoczatek >= _wierszKoniec
                || _kolumnaPoczatek < 0 || _kolumnaKoniec > _oryginalnaMacierz.ksztalt()[1] || _kolumnaPoczatek >= _kolumnaKoniec) {
            throw new ZlyIndeks();
        }

        oryginalnaMacierz = _oryginalnaMacierz;
        wierszPoczatek = _wierszPoczatek;
        wierszKoniec = _wierszKoniec;
        kolumnaPoczatek = _kolumnaPoczatek;
        kolumnaKoniec = _kolumnaKoniec;
    }

    // przycinamy macierz oryginalną do odpowiednich wierszy i kolumn
    private static double[][] przytnijMacierz(Macierz m, int wierszPoczatek, int wierszKoniec, int kolumnaPoczatek, int kolumnaKoniec) {
        int wysokosc = wierszKoniec - wierszPoczatek;
        int szerokosc = kolumnaKoniec - kolumnaPoczatek;
        double[][] nowaWartosc = new double[wysokosc][szerokosc];

        for (int i = 0; i < wysokosc; i++) {
            for (int j = 0; j < szerokosc; j++) {
                nowaWartosc[i][j] = m.daj(i + wierszPoczatek, j + kolumnaPoczatek);
            }
        }

        return nowaWartosc;
    }

    @Override
    public Tablica kopia() {
        return new MacierzDane(przytnijMacierz(this, wierszPoczatek, wierszKoniec, kolumnaPoczatek, kolumnaKoniec));
    }

    @Override
    public void przypisz(Tablica t) {
        oryginalnaMacierz.przypisz(t);
    }

    @Override
    public int[] ksztalt() {
        return new int[] {
                wierszKoniec - wierszPoczatek,
                kolumnaKoniec - kolumnaPoczatek
        };
    }



    @Override
    public double daj(int... indeksy) {
        if (indeksy.length != 2)
            throw new BledneZakresy();

        int i = indeksy[0], j = indeksy[1];

        if (i < 0 || i > (wierszKoniec - wierszPoczatek)
                || j < 0 || j > (kolumnaKoniec - kolumnaPoczatek)) {
            throw new BledneZakresy();
        }

        return oryginalnaMacierz.daj(i + wierszPoczatek, j + kolumnaPoczatek);
    }

    @Override
    public void ustaw(double wartosc, int... indeksy) {
        if (indeksy.length != 2)
            throw new BledneZakresy();

        int i = indeksy[0], j = indeksy[1];

        if (i < 0 || i > (wierszKoniec - wierszPoczatek)
                || j < 0 || j > (kolumnaKoniec - kolumnaPoczatek)) {
            throw new BledneZakresy();
        }

        oryginalnaMacierz.ustaw(wartosc, i + wierszPoczatek, j + kolumnaPoczatek);
    }

}
