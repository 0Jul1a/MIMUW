package tablice;

public class MacierzDane extends Macierz {

    public MacierzDane(double[][] macierzWartosc) {
        super(macierzWartosc);
    }

    @Override
    public Tablica kopia() {

        double[][] kopia = new double[getLiczbaWierszy()][getLiczbaKolumn()];

        for (int i = 0; i < getLiczbaWierszy(); i++) {
            for (int j = 0; j < getLiczbaKolumn(); j++) {
                kopia[i][j] = daj(i, j);
            }
        }

        return new MacierzDane(kopia);
    }

    @Override
    public void przypisz(Tablica t) {

        if (t.wymiar() > wymiar()) {
            throw new NiezgodnoscRozmiarow();
        } else if (t.wymiar() == 0) {  // przypisujemy skalar

            double wartosc = ((Skalar) t).getSkalarWartosc();
            for (int i = 0; i < getLiczbaWierszy(); i++)
                for (int j = 0; j < getLiczbaKolumn(); j++)
                    ustaw(wartosc, i, j);

        } else if (t.wymiar() == 1) { // przypisujemy wektor

            Wektor w = (Wektor) t;

            if (w.czyWektorPoziomy() && w.liczbaElementow() == getLiczbaKolumn()) {     // przypisujemy wektor poziomy

                for (int i = 0; i < getLiczbaWierszy(); i++)
                    for (int j = 0; j < getLiczbaKolumn(); j++)
                        ustaw(w.daj(j), i, j);

            } else if (w.liczbaElementow() == getLiczbaWierszy()) {                     // przypisujemy wektor pionowy

                for (int i = 0; i < getLiczbaWierszy(); i++)
                    for (int j = 0; j < getLiczbaKolumn(); j++)
                        ustaw(w.daj(i), i, j);

            } else {
                throw new NiezgodnoscRozmiarow();
            }
        } else if (t.wymiar() == 2) { // przypisujemy macierz

            Macierz m = (Macierz) t;
            if (m.ksztalt()[0] != getLiczbaWierszy() || m.ksztalt()[1] != getLiczbaKolumn())
                throw new NiezgodnoscRozmiarow();

            for (int i = 0; i < getLiczbaWierszy(); i++)
                for (int j = 0; j < getLiczbaKolumn(); j++)
                    ustaw(m.daj(i, j), i, j);
        }
    }

    @Override
    public double daj(int... indeksy) {
        if (indeksy.length != 2) {
            throw new ZlyIndeks();
        }

        int i = indeksy[0], j = indeksy[1];

        if (i < 0 || i > getLiczbaWierszy() || j < 0 || j > getLiczbaKolumn()) {
            throw new BledneZakresy();
        }

        return daj(i, j);
    }

    @Override
    public void ustaw(double wartosc, int... indeksy) {
        if (indeksy.length != 2) {
            throw new ZlyIndeks();
        }

        int i = indeksy[0], j = indeksy[1];

        if (i < 0 || i > getLiczbaWierszy()
                || j < 0 || j > getLiczbaKolumn()) {
            throw new BledneZakresy();
        }

        ustaw(wartosc, i, j);
    }


    public double daj(int i, int j) {
        return getMacierzWartosc()[i][j];
    }


    public void ustaw(double wartosc, int i, int j) {
        getMacierzWartosc()[i][j] = wartosc;
    }

}
