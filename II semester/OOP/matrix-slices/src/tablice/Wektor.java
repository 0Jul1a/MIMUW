package tablice;

public abstract class Wektor extends Tablica {

    private double[] wektorWartosc;
    private boolean wektorPoziomy;

    public Wektor (double[] _wektorWartosc, boolean _wektorPoziomy) {
        wektorWartosc = _wektorWartosc;
        wektorPoziomy = _wektorPoziomy;
    }

    public Tablica suma(Macierz m) {
        return m.suma(this);
    }

    public Tablica suma(Wektor w) {
        if (w.wektorWartosc.length != wektorWartosc.length) {
            throw new NiezgodnoscRozmiarow();
        } else if(w.wektorPoziomy != wektorPoziomy) {
            throw new NiezgodnoscRozmiarow();
        }

        return new WektorDane(dodajDoWektoraWektor(w , liczbaElementow()), czyWektorPoziomy());
    }

    private double[] dodajDoWektoraWektor(Wektor w , int zakres) {
        double[] nowaWartosc  = new double[zakres];

        for (int i = 0; i < zakres; i++)
            nowaWartosc[i] = daj(i) + w.daj(i);

        return nowaWartosc;
    }

    private double[] dodajDoWektoraSkalar(Skalar s , int zakres) {
        double[] nowaWartosc  = new double[zakres];

        for (int i = 0; i < zakres; i++)
            nowaWartosc[i] = daj(i) + s.daj();

        return nowaWartosc;
    }

    private double[] wektorRazyWektor(Wektor w , int zakres) {
        double[] nowaWartosc  = new double[zakres];

        for (int i = 0; i < zakres; i++)
            nowaWartosc[i] = daj(i) + w.daj(i);

        return nowaWartosc;
    }

    private double[] wektoraRazySkalar(Skalar s , int zakres) {
        double[] nowaWartosc  = new double[zakres];

        for (int i = 0; i < zakres; i++)
            nowaWartosc[i] = daj(i) * s.daj();

        return nowaWartosc;
    }

    private double[] wektorPoziomyRazyMacierz(Macierz m) {

        double[] nowaWartosc = new double[m.getLiczbaKolumn()];
        for (int i = 0; i < m.getLiczbaKolumn(); i++) {
            double sum = 0;
            for (int k = 0; k < liczbaElementow(); k++) {
                sum += m.daj(k, i) * daj(k);
            }
            nowaWartosc[i] = sum;
        }

        return nowaWartosc;
    }

    protected double[][] wektorMnozenieMacierzObustronne(Macierz m, Wektor w) {

        double[][] nowaWartosc = new double[m.getLiczbaWierszy()][w.liczbaElementow()];

        for (int i = 0; i < m.getLiczbaWierszy(); i++) {

            for (int j = 0; j < w.liczbaElementow(); j++) {
                double suma = 0;
                for (int k = 0; k < m.getLiczbaKolumn(); k++) {
                    suma += m.daj(i, k) * w.daj(j);
                }
                nowaWartosc[i][j] = suma;
            }
        }
        return nowaWartosc;
    }

    public Tablica suma(Skalar s) {
        return new WektorDane(dodajDoWektoraSkalar(s, wektorWartosc.length), wektorPoziomy);
    }

    public void dodaj(Macierz m) {
        m.dodaj(this);
    }

    public void dodaj(Wektor w) {

        if (w.wektorWartosc.length != wektorWartosc.length) {
            throw new NiezgodnoscRozmiarow();
        }
        if(w.wektorPoziomy != wektorPoziomy) {
            throw new NiezgodnoscRozmiarow();
        }
        wektorWartosc = dodajDoWektoraWektor(w ,liczbaElementow());
    }

    public void dodaj(Skalar s) {
        wektorWartosc = dodajDoWektoraSkalar(s, wektorWartosc.length);
    }

    public Tablica negacja() {
        return this.iloczyn(new SkalarDane(-1.0));
    }

    public void zaneguj() {
        this.przemnoz(new SkalarDane(-1));
    }

    public Tablica iloczyn(Macierz m) {

        if (czyWektorPoziomy()) {
            if (liczbaElementow() != m.getLiczbaWierszy())
                throw new NiezgodnoscRozmiarow();

            return new WektorDane(wektorPoziomyRazyMacierz(m), true);
        } else {
            if (m.getLiczbaWierszy() != 1)
                throw new NiezgodnoscRozmiarow();

            return new MacierzDane( wektorMnozenieMacierzObustronne(m, this));
        }
    }

    public Tablica iloczyn(Skalar s) {
        return new WektorDane(wektoraRazySkalar(s, liczbaElementow()), wektorPoziomy);
    }

    public Tablica iloczyn(Wektor w) {
        double tmp = 0;

        for (int i = 0; i < liczbaElementow(); i++) {
            tmp += daj(i) * w.daj(i);
        }

        if (wektorPoziomy == w.wektorPoziomy) {
            if (w.wektorWartosc.length != wektorWartosc.length) {
                throw new NiezgodnoscRozmiarow();
            }

            return new SkalarDane(tmp);
        } else {
            if (wektorPoziomy && !w.wektorPoziomy) {

                double[][] nowaWartosc = new double[1][1];
                nowaWartosc[0][0] = tmp;
                return new MacierzDane(nowaWartosc);
            }

            double[][] nowaWartosc = new double[liczbaElementow()][w.liczbaElementow()];

            for (int i = 0; i < liczbaElementow(); i++) {
                for (int j = 0; j < w.liczbaElementow(); j++) {
                    nowaWartosc[i][j] = daj(i) * w.daj(j);
                }
            }
            return new MacierzDane(nowaWartosc);
        }
    }

    public void przemnoz(Macierz m) {

        if (czyWektorPoziomy()) {
            if (liczbaElementow() != m.getLiczbaWierszy())
                throw new NiezgodnoscRozmiarow();

            wektorWartosc = wektorPoziomyRazyMacierz(m);
        } else{
                throw new NiezgodnoscRozmiarow();
            }
    }

    public void przemnoz(Skalar s) {
        wektorWartosc = wektoraRazySkalar(s, liczbaElementow());
    }

    public void przemnoz(Wektor w) {
        throw new NiezgodnoscRozmiarow(); // poniewaz wynikiem jest albo skalar albo macierz
    }

    @Override
    public int wymiar() {
        return 1;
    }

    @Override
    public int liczbaElementow() {
        return wektorWartosc.length;
    }

    @Override
    public int[] ksztalt() {
        int[] tmp = new int[1];
        tmp[0] = liczbaElementow();
        return tmp;
    }

    @Override
    public void transponuj() {
        wektorPoziomy = !wektorPoziomy;
    }

    @Override
    public String toString() {
        String res;
        if (wektorPoziomy) {
            res =  "[ ";
            for (int i = 0; i < wektorWartosc.length; i++) {
                if (i > 0)
                    res += "  ";
                res += wektorWartosc[i];
            }
            res += " ]\n";
        } else {
            res = "⎡ ";
            res += wektorWartosc[0];
            res += " ⎤\n";

            for (int i = 1; i < wektorWartosc.length - 1; i++) {
                res += "| ";
                res += wektorWartosc[i];
                res += " |\n";
            }

            res += "⎣ ";
            res += wektorWartosc[wektorWartosc.length -1];
            res += " ⎦\n";

        }
        return res;
    }

    public double[] getWektorWartosc() {
        return wektorWartosc;
    }

    public boolean czyWektorPoziomy() {
        return wektorPoziomy;
    }

    public void ustawWektor(boolean orientacja) {
        wektorPoziomy = orientacja;
    }

    @Override
    public boolean equals(Object obj) {

        if (obj instanceof Wektor) {
            if (wektorPoziomy != ((Wektor) obj).czyWektorPoziomy())
                return false;
            if (wektorWartosc.length != ((Wektor) obj).liczbaElementow())
                return false;

            for (int i = 0; i < wektorWartosc.length; i++) {
                if (wektorWartosc[i] != ((Wektor) obj).wektorWartosc[i])
                    return false;
            }
            return true;
        }
        return false;
    }

    public Wektor wycinek(int... zakres) {
        return new WektorWycinek(this, zakres[0], zakres[1]);
    }

}
