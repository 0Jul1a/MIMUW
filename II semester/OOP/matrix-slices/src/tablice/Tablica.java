package tablice;

public abstract class Tablica {

    public abstract int wymiar();

    public abstract Tablica kopia();

    public abstract String toString();

    public abstract int liczbaElementow();

    public abstract int[] ksztalt();

    public abstract void transponuj();

    public abstract void przypisz(Tablica t);

    public abstract double daj(int... indeksy);

    public abstract void ustaw(double wartosc, int... indeksy);

    public abstract Tablica wycinek(int... zakres);


}
