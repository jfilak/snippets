import java.util.ArrayList;
import java.lang.IndexOutOfBoundsException;

public class Palindroms
{
    static class Matrix
    {
        private int[] m;
        private int r;
        private int c;

        public Matrix(int rows, int columns)
        {
            this.r = rows;
            this.c = columns;
            this.m = new int[this.r * this.c];
        }

        private void checkBounds(int x, int y) throws IndexOutOfBoundsException
        {
            if (x < 0 || y < 0 || x >= this.c || y >= this.r) {
                throw new IndexOutOfBoundsException(String.format("%d !e <0, %d> OR %d !e <0, %d>", x, this.c, y, this.r));
            }
        }

        int get(int x, int y) throws IndexOutOfBoundsException
        {
            this.checkBounds(x, y); 
            return this.m[y * this.c + x];
        }

        Matrix set(int x, int y, int v) throws IndexOutOfBoundsException
        {
            this.checkBounds(x, y); 
            this.m[y * this.c + x] = v;
            return this;
        }
    }

    static String[] findAllInAString(String input)
    {
        Palindroms.Matrix m = new Palindroms.Matrix(input.length() + 1, input.length() + 1); 

        for (int x = 0; x <= input.length(); ++x) {
           m.set(x, 0, 0).set(0, x, 0);
        }

        for (int x = 1; x <= input.length(); ++x) {
            for (int y = 1; y <= input.length(); ++y) {
                int prev = m.get(x - 1, y - 1);
                char f = input.charAt(x - 1);
                char s = input.charAt(input.length() - y);
                int cur = (f == s ? prev + 1 : 0);
                m.set(x, y, cur);
            }
        }

        ArrayList<String> res = new ArrayList<String>();
        for (int x = input.length(); x != 0; --x) {
            int y = input.length() - x + 1;
            int v = m.get(x, y);
            int s = x - 1 - (v - 1);
            int e = s + ((v * 2) - 1);
            res.add(input.substring(s, e));
            
            ++y;
            if (y <= input.length()) {
                v = m.get(x, y);
                if (v > 0) {
                    s = (x - (v - 1)) - 1;
                    e = s + (v - 1) * 2;
                    res.add(input.substring(s, e));
                }
            }
        }

        return res.toArray(new String[res.size()]);
    }
}
