package tablice;

public abstract class Skalar extends Tablica {

    private double skalarWartosc;

    public Skalar (double _skalarWartosc) {
        skalarWartosc = _skalarWartosc;
    }

    public Tablica negacja() {
        return this.iloczyn(new SkalarDane(-1.0));
    }

    public void zaneguj() {
        this.przemnoz(new SkalarDane(-1));
    }

    public Tablica suma(Skalar s) {
        return new SkalarDane(s.skalarWartosc + skalarWartosc);
    }

    public Tablica suma(Wektor w) {
        return w.suma(this);
    }

    public Tablica suma(Macierz m) {
        return m.suma(this);
    }

    public void dodaj(Skalar s) {
         skalarWartosc += s.skalarWartosc;
    }

    public void dodaj(Wektor w) {
        throw new NiezgodnoscRozmiarow();
    }

    public void dodaj(Macierz m) {
        throw new NiezgodnoscRozmiarow();
    }

    public Tablica iloczyn(Macierz m) {
        return m.iloczyn(this);
    }

    public Tablica iloczyn(Wektor w) {
        return w.iloczyn(this);
    }

    public Tablica iloczyn(Skalar s) {
        return new SkalarDane(skalarWartosc * s.skalarWartosc);
    }

    public void przemnoz(Macierz m) {
        throw new NiezgodnoscRozmiarow();
    }

    public void przemnoz(Wektor w) {
        throw new NiezgodnoscRozmiarow();
    }

    public void przemnoz(Skalar s) {
        skalarWartosc *= s.skalarWartosc;
    }

    @Override
    public int liczbaElementow() {
        return 1;
    }

    @Override
    public int[] ksztalt() {
        return new int[]{};
    }

    @Override
    public void transponuj() {
    } // nie robi nic

    @Override
    public String toString() {
        return "[" + skalarWartosc + "]";
    }

    public double getSkalarWartosc() {
        return skalarWartosc;
    }

    @Override
    public int wymiar() {
        return 0;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof Skalar)
            return ((Skalar) obj).skalarWartosc == skalarWartosc;

        return false;
    }

    public Skalar wycinek(int... zakres) {
        return new SkalarWycinek(this);
    }

}
