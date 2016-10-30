import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Arrays;

public class AppEntry
{
    public static void main(String[] args) throws java.io.IOException
    {
        BufferedReader rd = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            String s = rd.readLine();

            String[] palindroms = Palindroms.findAllInAString(s);
            Arrays.sort(palindroms);

            for (String p : palindroms) {
                System.out.printf("- %s\n", p);
            }
        }
    }
}
