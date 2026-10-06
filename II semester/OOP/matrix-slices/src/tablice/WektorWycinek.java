package tablice;

import java.util.Arrays;

public class WektorWycinek extends Wektor {

    private final Wektor oryginalnyWektor;
    private final int poczatek;
    private final int koniec;

    public WektorWycinek(Wektor _oryginalnyWektor, int _poczatek, int _koniec) {
        super(Arrays.copyOfRange(_oryginalnyWektor.getWektorWartosc(), _poczatek, _koniec + 1), _oryginalnyWektor.czyWektorPoziomy());

        if (_poczatek < 0 || _koniec > _oryginalnyWektor.ksztalt()[0] || _poczatek >= _koniec) {
            throw new ZlyIndeks();
        }

        oryginalnyWektor = _oryginalnyWektor;
        poczatek = _poczatek;
        koniec = _koniec;
    }

    @Override
    public Tablica kopia() {
        double[] nowaWartoscc = Arrays.copyOfRange(oryginalnyWektor.getWektorWartosc(), poczatek, koniec);
        return new WektorDane(nowaWartoscc, czyWektorPoziomy());
    }

    @Override
    public void przypisz(Tablica t) {
        oryginalnyWektor.przypisz(t);
    }

    @Override
    public double daj(int... indeksy) {
        if (indeksy.length != 1)
            throw new BledneZakresy();

        int i = indeksy[0];

        if (i < 0 || i > (koniec - poczatek))
            throw new BledneZakresy();

        return oryginalnyWektor.daj(i + poczatek);                     // zwracamy odpowiednio przesuniętą wartość
    }

    @Override
    public void ustaw(double wartosc, int... indeksy) {
        if (indeksy.length != 1)
            throw new BledneZakresy();

        int i = indeksy[0];

        if (i < 0 || i > (koniec - poczatek))
            throw new BledneZakresy();

        oryginalnyWektor.ustaw(wartosc, i + poczatek);
    }

    @Override
    public int[] ksztalt() {
        return new int[] { koniec - poczatek };
    }

}

