package tablice;

import javax.crypto.Mac;

public abstract class Macierz extends Tablica {

    private double[][] macierzWartosc;
    private int liczbaWierszy;
    private int liczbaKolumn;

    public Macierz (double[][] _macierzDane) {
        macierzWartosc = _macierzDane;
        liczbaKolumn = macierzWartosc[0].length;
        liczbaWierszy = macierzWartosc.length;
    }

    public Tablica negacja() {
        return this.iloczyn(new SkalarDane(-1.0));
    }

    public void zaneguj() {
        this.przemnoz(new SkalarDane(-1));
    }

    private double[][] dodajDoMacierzyWektorPionowy(Wektor w, int zakres1, int zakres2) {
        double[][] nowaWartosc = new double[liczbaWierszy][liczbaKolumn];

        for (int i = 0; i < zakres1; i++) {
            for (int j = 0; j < zakres2; j++) {
                nowaWartosc[i][j] = macierzWartosc[i][j] + w.daj(i);
            }
        }

        return nowaWartosc;
    }

    private double[][] dodajDoMacierzyWektorPoziomy(Wektor w, int zakres1, int zakres2) {
        double[][] nowaWartosc = new double[liczbaWierszy][liczbaKolumn];

            for (int j = 0; j < zakres1; j++) {
                for (int i = 0; i < zakres2; i++) {
                    nowaWartosc[i][j] = macierzWartosc[i][j] + w.daj(j);
                }
            }

        return nowaWartosc;
    }

    private double[][] dodajDoMacierzyMacierz(Macierz m) {
        double[][] nowaWartosc = new double[liczbaWierszy][liczbaKolumn];

        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < liczbaKolumn; j++) {
                nowaWartosc[i][j] = macierzWartosc[i][j] + m.daj(i, j);
            }
        }
            return nowaWartosc;
    }

    private double[][] macierzRazyMacierz(Macierz m) {
        double[][] nowaWartosc = new double[liczbaWierszy][m.liczbaKolumn];

        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < m.liczbaKolumn; j++) {
                double sum = 0;
                for (int k = 0; k < liczbaKolumn; k++) {
                    sum += m.daj(k, j) * daj(i, k);
                }
                nowaWartosc[i][j] = sum;
            }
        }

        return nowaWartosc;
    }

    private double[] macierzRazyWektorPionowy(Wektor w) {
        double[] nowaWartosc = new double[liczbaWierszy];

        for (int i = 0; i < liczbaWierszy; i++) {
            double sum = 0;
            for (int k = 0; k < liczbaKolumn; k++) {
                sum += daj(i, k) * w.daj(k);
            }
            nowaWartosc[i] = sum;
        }

        return nowaWartosc;
    }

    public Tablica suma(Macierz m) {
        if (liczbaKolumn != m.liczbaKolumn || liczbaWierszy != m.liczbaWierszy)
            throw new NiezgodnoscRozmiarow();

        return new MacierzDane(dodajDoMacierzyMacierz(m));
    }

    public Tablica suma(Wektor w) {
        double[][] nowaWartosc;

        if (w.czyWektorPoziomy()) {
            if (w.liczbaElementow() != liczbaKolumn)
                throw new NiezgodnoscRozmiarow();

            nowaWartosc = dodajDoMacierzyWektorPoziomy(w, liczbaKolumn, liczbaWierszy);
        } else {
            if (w.liczbaElementow() != liczbaWierszy)
                throw new NiezgodnoscRozmiarow();

            nowaWartosc = dodajDoMacierzyWektorPionowy(w, liczbaWierszy, liczbaKolumn);
        }

        return new MacierzDane(nowaWartosc);
    }

    public Tablica suma(Skalar s) {
        double[][] nowaWartosc = new double[liczbaWierszy][liczbaKolumn];

        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < liczbaKolumn; j++) {
                nowaWartosc[i][j] = daj(i, j) + s.daj();
            }
        }

        return new MacierzDane(nowaWartosc);
    }

    public void dodaj(Macierz m) {
        if (liczbaKolumn != m.liczbaKolumn || liczbaWierszy != m.liczbaWierszy)
            throw new NiezgodnoscRozmiarow();

        macierzWartosc = dodajDoMacierzyMacierz(m);
    }

    public void dodaj(Wektor w) {
        if (w.czyWektorPoziomy()) {
            if (w.getWektorWartosc().length != liczbaKolumn)
                throw new NiezgodnoscRozmiarow();

            macierzWartosc = dodajDoMacierzyWektorPoziomy(w, liczbaKolumn, liczbaWierszy);
        } else {
            if (w.getWektorWartosc().length != liczbaWierszy)
                throw new NiezgodnoscRozmiarow();

            macierzWartosc = dodajDoMacierzyWektorPionowy(w, liczbaWierszy, liczbaKolumn);
        }
    }

    public void dodaj(Skalar s) {
        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < liczbaKolumn; j++) {
                macierzWartosc[i][j] = daj(i, j) + s.daj();
            }
        }
    }

    public Tablica iloczyn(Macierz m) {
        if (liczbaKolumn != m.liczbaWierszy)
            throw new NiezgodnoscRozmiarow();

        return new MacierzDane(macierzRazyMacierz(m));
    }

    public Tablica iloczyn(Wektor w) {
        if (!w.czyWektorPoziomy()) {
            if (liczbaKolumn != w.liczbaElementow())
                throw new NiezgodnoscRozmiarow();

            return new WektorDane(macierzRazyWektorPionowy(w), false);
        } else {
            if (liczbaWierszy != 1)
                throw new NiezgodnoscRozmiarow();

            return new MacierzDane(w.wektorMnozenieMacierzObustronne(this, w));

        }
    }

    public Tablica iloczyn(Skalar s) {

        double[][] nowaWartosc = new double[liczbaWierszy][liczbaKolumn];

        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < liczbaKolumn; j++) {
                nowaWartosc[i][j] = daj(i, j) * s.daj();
            }
        }
        return new MacierzDane(nowaWartosc);
    }

    public void przemnoz(Macierz m) {
        if (liczbaKolumn != m.liczbaWierszy)
            throw new NiezgodnoscRozmiarow();

        macierzWartosc = macierzRazyMacierz(m);
    }

    public void przemnoz(Wektor w) {
        if (w.czyWektorPoziomy()) {
            if (liczbaKolumn != 1)
                throw new NiezgodnoscRozmiarow();

            double[][] nowaWartosc = w.wektorMnozenieMacierzObustronne(this, w);
            macierzWartosc = nowaWartosc;
        } else {
            if (liczbaKolumn != w.liczbaElementow())
                throw new NiezgodnoscRozmiarow();

            double[] nowaWartosc = macierzRazyWektorPionowy(w);
            for (int i = 0; i < liczbaWierszy; i++)
                ustaw(nowaWartosc[i], i, 0);
        }
    }

    public void przemnoz(Skalar s) {

        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < liczbaKolumn; j++) {
                macierzWartosc[i][j] = daj(i, j) * s.daj();
            }
        }
    }

    @Override
    public int liczbaElementow() {
        return liczbaWierszy * liczbaKolumn;
    }

    @Override
    public int[] ksztalt() {

        int[] tmp = new int[2];
        tmp[0] = liczbaWierszy;
        tmp[1] = liczbaKolumn;
        return tmp;

    }

    public double[][] getMacierzWartosc() {
        return macierzWartosc;
    }

    public int getLiczbaKolumn() {
        return liczbaKolumn;
    }

    public int getLiczbaWierszy() {
        return liczbaWierszy;
    }

    @Override
    public void transponuj() {

        double[][] tmp = new double[liczbaKolumn][liczbaWierszy];

        for (int i = 0; i < liczbaWierszy; i++) {
            for (int j = 0; j < liczbaKolumn; j++) {
                tmp[j][i] = macierzWartosc[i][j];
            }
        }

        int tmpWierszy = liczbaWierszy;
        liczbaWierszy = liczbaKolumn;
        liczbaKolumn = tmpWierszy;
        macierzWartosc = tmp;
    }


    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < liczbaWierszy; i++) {
            if (i == liczbaWierszy - 1)
                sb.append("⎣ ");
            else if (i == 0)
                sb.append("⎡ ");
            else
                sb.append("| ");

            for (int j = 0; j < liczbaKolumn; j++) {
                sb.append(macierzWartosc[i][j]);
                if (j < liczbaKolumn - 1) {
                    sb.append("  ");
                }
            }
            if (i < 1)
                sb.append(" ⎤\n");
            else if (i == liczbaWierszy - 1) {
                sb.append(" ⎦\n");
            } else
                sb.append(" |\n");
        }
        return sb.toString();
    }

    @Override
    public int wymiar() {
        return 2;
    }

    @Override
    public boolean equals(Object obj) {

        if (obj instanceof Macierz) {
            if (liczbaWierszy != ((Macierz) obj).getLiczbaWierszy() || liczbaKolumn != ((Macierz) obj).getLiczbaKolumn())
                throw new NiezgodnoscRozmiarow();
            for (int i = 0; i < liczbaWierszy; i++) {
                for (int j = 0; j < liczbaKolumn; j++) {
                    if (macierzWartosc[i][j] != ((Macierz) obj).macierzWartosc[i][j])
                        return false;
                }
            }
            return true;
        }
        return false;
    }

    public Macierz wycinek(int... zakres) {

        return new MacierzWycinek(this, zakres[0], zakres[1], zakres[2], zakres[3]);
    }

}
