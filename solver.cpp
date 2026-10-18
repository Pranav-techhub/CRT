
#include <iostream>
using namespace std;

int gcdSimple(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }

    return a;
}

int positiveMod(int a, int b)
{
    return ((a % b) + b) % b;
}

int main()
{
    string vehicle;
    int departure, n;

    cin >> vehicle >> departure >> n;

    int cycle[10], offset[10], greenStart[10];
    int greenDuration[10], travel[10];

    int target[10];
    int residue[10];
    int elapsed = 0;

    for (int i = 0; i < n; i++)
    {
        cin >> cycle[i] >> offset[i]
            >> greenStart[i] >> greenDuration[i]
            >> travel[i];

        target[i] = positiveMod(
            greenStart[i] + (greenDuration[i] - 1) / 2,
            cycle[i]
        );

        residue[i] = positiveMod(
            offset[i] + target[i] - elapsed,
            cycle[i]
        );

        elapsed += travel[i];
    }

    // Combine the congruences using CRT
    long long result = residue[0];
    long long currentModulus = cycle[0];
    bool possible = true;

    for (int i = 1; i < n; i++)
    {
        int newModulus = cycle[i];
        int common = gcdSimple(
            (int)currentModulus,
            newModulus
        );

        if ((residue[i] - result) % common != 0)
        {
            possible = false;
            break;
        }

        long long nextModulus =
            currentModulus / common * newModulus;

        long long candidate = result;

        while (candidate < nextModulus)
        {
            if (candidate % newModulus == residue[i])
            {
                break;
            }

            candidate += currentModulus;
        }

        if (candidate >= nextModulus)
        {
            possible = false;
            break;
        }

        result = candidate;
        currentModulus = nextModulus;
    }

    cout << "{";
    cout << "\"vehicle\":\"" << vehicle << "\",";
    cout << "\"departure\":" << departure << ",";
    cout << "\"crtPossible\":"
         << (possible ? "true" : "false") << ",";

    if (possible)
    {
        cout << "\"crtResidue\":" << result << ",";
        cout << "\"crtModulus\":" << currentModulus << ",";
        cout << "\"crtTime\":" << result << ",";
        cout << "\"crtExplanation\":\"Common solution exists\",";
    }
    else
    {
        cout << "\"crtResidue\":null,";
        cout << "\"crtModulus\":null,";
        cout << "\"crtTime\":null,";
        cout << "\"crtExplanation\":\"No common solution exists\",";
    }

    // Check the actual signal at the selected departure time
    bool allGreen = true;
    elapsed = 0;

    cout << "\"signals\":[";

    for (int i = 0; i < n; i++)
    {
        int phase = positiveMod(
            departure + elapsed - offset[i],
            cycle[i]
        );

        int difference = positiveMod(
            phase - greenStart[i],
            cycle[i]
        );

        bool green = difference < greenDuration[i];

        if (!green)
        {
            allGreen = false;
        }

        if (i > 0)
        {
            cout << ",";
        }

        cout << "{";
        cout << "\"name\":\"Intersection " << i + 1 << "\",";
        cout << "\"arrival\":" << departure + elapsed << ",";
        cout << "\"phase\":" << phase << ",";
        cout << "\"cycle\":" << cycle[i] << ",";
        cout << "\"green\":" << (green ? "true" : "false") << ",";
        cout << "\"status\":\""
             << (green ? "Green" : "Red") << "\"";
        cout << "}";

        elapsed += travel[i];
    }

    cout << "],";
    cout << "\"allGreen\":" << (allGreen ? "true" : "false") << ",";
    cout << "\"routeStatus\":\""
         << (allGreen ? "All signals green" : "Some signals red")
         << "\"";

    cout << "}";

    return 0;
}
