
#include <iostream>
#include <string>
using namespace std;

int gcdSimple(int a, int b)
{
    while (b != 0)
    {
        int r = a % b;
        a = b;
        b = r;
    }
    return a;
}

int positiveMod(int value, int mod)
{
    value = value % mod;

    if (value < 0)
        value = value + mod;

    return value;
}

void printTime(int seconds)
{
    seconds = positiveMod(seconds, 86400);

    int hour = seconds / 3600;
    int minute = (seconds % 3600) / 60;

    if (hour < 10) cout << "0";
    cout << hour << ":";

    if (minute < 10) cout << "0";
    cout << minute;
}

int main()
{
    string vehicle;
    int departure, n;

    cin >> vehicle >> departure >> n;

    if (n < 1 || n > 20)
    {
        cout << "{}";
        return 0;
    }

    int cycle[20], offset[20];
    int greenStart[20], greenDuration[20];
    int travel[20], target[20], remainder[20];
    int arrival[20], phase[20];
    bool green[20];

    int commonPeriod = 1;

    for (int i = 0; i < n; i++)
    {
        cin >> cycle[i] >> offset[i]
            >> greenStart[i] >> greenDuration[i]
            >> travel[i];

        if (cycle[i] <= 0 || greenDuration[i] <= 0 ||
            greenDuration[i] > cycle[i] || travel[i] < 0)
        {
            cout << "{}";
            return 0;
        }

        int g = gcdSimple(commonPeriod, cycle[i]);

        commonPeriod = (commonPeriod / g) * cycle[i];
    }

    // Calculate the target phase for each signal.
    int totalTravel = 0;

    for (int i = 0; i < n; i++)
    {
        target[i] = positiveMod(
            greenStart[i] + (greenDuration[i] - 1) / 2,
            cycle[i]);

        remainder[i] = positiveMod(
            offset[i] + target[i] - totalTravel,
            cycle[i]);

        totalTravel = totalTravel + travel[i];
    }

    // Start with the first congruence.
    int result = remainder[0];
    int currentModulus = cycle[0];
    bool possible = true;

    // Combine the remaining congruences using CRT.
    for (int i = 1; i < n; i++)
    {
        int g = gcdSimple(currentModulus, cycle[i]);

        // Check whether the equations are compatible.
        if (positiveMod(result - remainder[i], g) != 0)
        {
            possible = false;
            break;
        }

        int newModulus = (currentModulus / g) * cycle[i];
        int candidate = result;
        bool found = false;

        // Find a value satisfying both equations.
        while (candidate < newModulus)
        {
            if (positiveMod(candidate - remainder[i],
                            cycle[i]) == 0)
            {
                found = true;
                break;
            }

            candidate = candidate + currentModulus;
        }

        if (!found)
        {
            possible = false;
            break;
        }

        result = candidate;
        currentModulus = newModulus;
    }

    // Check the entered departure time at every intersection.
    totalTravel = 0;
    bool allGreen = true;

    for (int i = 0; i < n; i++)
    {
        arrival[i] = departure + totalTravel;

        phase[i] = positiveMod(
            arrival[i] - offset[i], cycle[i]);

        int difference = positiveMod(
            phase[i] - greenStart[i], cycle[i]);

        green[i] = (difference < greenDuration[i]);

        if (!green[i])
            allGreen = false;

        totalTravel = totalTravel + travel[i];
    }

    // Display results in JSON format for the website.
    cout << "{";

    cout << "\"departure\":\"";
    printTime(departure);
    cout << "\",";

    cout << "\"allGreen\":"
         << (allGreen ? "true" : "false") << ",";

    cout << "\"routeStatus\":\""
         << (allGreen ? "All signals green"
                      : "Some signals are not green")
         << "\",";

    cout << "\"congruences\":\"";

    for (int i = 0; i < n; i++)
    {
        if (i > 0) cout << "; ";

        cout << "T ≡ " << remainder[i]
             << " (mod " << cycle[i] << ")";
    }

    cout << "\",";

    cout << "\"crtPossible\":"
         << (possible ? "true" : "false") << ",";

    cout << "\"crtResidue\":";

    if (possible)
        cout << result;
    else
        cout << "null";

    cout << ",\"crtModulus\":"
         << (possible ? currentModulus : commonPeriod) << ",";

    cout << "\"crtTime\":\"";

    if (possible)
        printTime(result);
    else
        cout << "Not found";

    cout << "\",";

    cout << "\"crtExplanation\":\"";

    if (possible)
    {
        cout << "CRT solution: T ≡ " << result
             << " (mod " << currentModulus
             << "). The target signal phases repeat after the combined period.";
    }
    else
    {
        cout << "No common solution exists because the congruences are incompatible.";
    }

    cout << "\",";

    cout << "\"signals\":[";

    for (int i = 0; i < n; i++)
    {
        if (i > 0) cout << ",";

        cout << "{";
        cout << "\"name\":\"Intersection " << i + 1 << "\",";
        cout << "\"arrival\":\"";
        printTime(arrival[i]);
        cout << "\",";
        cout << "\"phase\":" << phase[i] << ",";
        cout << "\"cycle\":" << cycle[i] << ",";
        cout << "\"green\":" << (green[i] ? "true" : "false") << ",";
        cout << "\"status\":\""
             << (green[i] ? "Green window"
                          : "Outside green window")
             << "\"}";
    }

    cout << "]}";

    return 0;
}
