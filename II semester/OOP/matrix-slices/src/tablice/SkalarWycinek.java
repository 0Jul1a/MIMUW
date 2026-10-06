package tablice;

public class SkalarWycinek extends Skalar {

    private final Skalar oryginalnySkalar;

    public SkalarWycinek(Skalar _oryginalnySkalar) {
        super(_oryginalnySkalar.getSkalarWartosc());
        oryginalnySkalar = _oryginalnySkalar;
    }

    @Override
    public Tablica kopia() {
        double kopia = getSkalarWartosc();
        return new SkalarDane(kopia);
    }

    @Override
    public void przypisz(Tablica t) {
        oryginalnySkalar.przypisz(t);
    }

    @Override
    public double daj(int... indeksy) {
        if (indeksy.length != 0) {
            throw new BledneZakresy();
        }
        return oryginalnySkalar.daj();
    }

    @Override
    public void ustaw(double wartosc, int... indeksy) {
        if (indeksy.length != 0) {
            throw new BledneZakresy();
        }
        oryginalnySkalar.ustaw(wartosc);
    }



}
