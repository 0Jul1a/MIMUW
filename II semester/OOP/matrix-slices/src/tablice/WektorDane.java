package tablice;

import java.util.Arrays;

public class WektorDane extends Wektor {

    public WektorDane(double[] _wektorWartosc, boolean _wektorPoziomy) {
        super(_wektorWartosc, _wektorPoziomy);
    }

    @Override
    public Tablica kopia() {
        double[] kopia = Arrays.copyOfRange(getWektorWartosc(), 0, liczbaElementow());
        return new WektorDane(kopia, czyWektorPoziomy());
    }

    @Override
    public void przypisz(Tablica t)  {

        if (t.wymiar() > wymiar()) {
            throw new NiezgodnoscRozmiarow();
        } else if (t.wymiar() == 0) {  // przypisujemy skalar

            double wartosc = ((Skalar) t).getSkalarWartosc();
            for (int i = 0; i < liczbaElementow(); i++)
                ustaw(wartosc, i);

        } else if (t.wymiar() == 1) {   // przypisujemy wektor

            Wektor w = (Wektor) t;
            if (w.liczbaElementow() != liczbaElementow())
                throw new NiezgodnoscRozmiarow();

            if (czyWektorPoziomy() != w.czyWektorPoziomy()) // wektory mają różną orientację więc musimy zmienić na taką samą
                ustawWektor(w.czyWektorPoziomy());

            for (int i = 0; i < liczbaElementow(); i++)
                ustaw(w.daj(i), i);
        }
    }

    @Override
    public double daj(int... indeksy){
        if (indeksy.length != 1) {
            throw new ZlyIndeks();
        }

        int i = indeksy[0];

        if (i < 0 || i > liczbaElementow())
            throw new BledneZakresy();

        return daj(i);
    }

    @Override
    public void ustaw(double wartosc, int... indeksy) {
        if (indeksy.length != 1) {
            throw new ZlyIndeks();
        }

        int i = indeksy[0];

        if (i < 0 || i > liczbaElementow())
            throw new BledneZakresy();

        ustaw(wartosc, i);
    }


    public double daj(int i){
        return getWektorWartosc()[i];
    }

    public void ustaw(double wartosc, int i) {
        getWektorWartosc()[i] = wartosc;
    }


}
