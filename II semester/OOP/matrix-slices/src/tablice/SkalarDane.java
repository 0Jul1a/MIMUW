package tablice;

public class SkalarDane extends Skalar {

    public SkalarDane(double _skalarWartosc) {
        super(_skalarWartosc);
    }

    @Override
    public Tablica kopia() {
        double kopia = daj();
        return new SkalarDane(kopia);
    }

    @Override
    public void przypisz(Tablica t) {
        if (t.wymiar() > wymiar())
            throw new NiezgodnoscRozmiarow();

        this.przemnoz(new SkalarDane(0));       // zerujemy obecny skalar
        this.dodaj((Skalar) t);                              // przypisujemy nowa wartosc

    }

    @Override
    public double daj(int... indeksy) {

        if (indeksy.length != 0) {
            throw new NiezgodnoscRozmiarow();
        }

        return daj();
    }

    @Override
    public void ustaw(double wartosc, int... indeksy) {

        if (indeksy.length != 0) {
            throw new ZlyIndeks();
        }

        ustaw(wartosc);
    }


    public double daj() {
        return getSkalarWartosc();
    }

    public void ustaw(double wartosc) {
        this.przypisz(new SkalarDane(wartosc));
    }
}
