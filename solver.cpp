
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <numeric>
#include <cstdio>
using namespace std;

long long mod(long long a, long long m) {
    long long r = a % m;
    return r < 0 ? r + m : r;
}

long long egcd(long long a, long long b,
               long long &x, long long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }

    long long x1, y1;
    long long g = egcd(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return g;
}

bool mergeCRT(long long &a, long long &m,
              long long b, long long n) {
    long long g = gcd(m, n);

    if (mod(b - a, g) != 0) {
        return false;
    }

    long long x, y;
    egcd(m / g, n / g, x, y);

    long long k = mod(((b - a) / g) * x, n / g);

    a = mod(a + m * k, m * (n / g));
    m = m * (n / g);

    return true;
}

string clockText(long long seconds) {
    seconds = mod(seconds, 86400);

    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%02lld:%02lld",
             seconds / 3600,
             (seconds % 3600) / 60);

    return buffer;
}

struct Signal {
    long long cycle;
    long long offset;
    long long start;
    long long duration;
    long long travel;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string vehicle;
    long long departure;
    int n;

    if (!(cin >> vehicle >> departure >> n)) {
        return 1;
    }

    vector<Signal> signals(n);

    for (auto &s : signals) {
        cin >> s.cycle >> s.offset
            >> s.start >> s.duration >> s.travel;
    }

    bool allGreen = true;
    bool possible = true;

    long long a = 0;
    long long m = 1;
    long long cumulativeTravel = 0;

    vector<string> rows;
    string equations;

    for (int i = 0; i < n; i++) {
        Signal s = signals[i];

        cumulativeTravel += s.travel;

        long long arrival = departure + cumulativeTravel;

        long long phase = mod(
            arrival - s.offset, s.cycle
        );

        bool green =
            mod(phase - s.start, s.cycle) < s.duration;

        if (!green) {
            allGreen = false;
        }

        long long target = mod(
            s.start + (s.duration - 1) / 2,
            s.cycle
        );

        long long residue = mod(
            s.offset + target - cumulativeTravel,
            s.cycle
        );

        if (i > 0) {
            equations += "; ";
        }

        equations += "T ≡ " + to_string(residue)
                  + " (mod " + to_string(s.cycle) + ")";

        if (possible &&
            !mergeCRT(a, m, residue, s.cycle)) {
            possible = false;
        }

        ostringstream row;

        row << "{\"name\":\"Intersection " << i + 1
            << "\",\"arrival\":\"" << clockText(arrival)
            << "\",\"phase\":" << phase
            << ",\"cycle\":" << s.cycle
            << ",\"green\":" << (green ? "true" : "false")
            << ",\"status\":\""
            << (green ? "Green window" : "Outside green window")
            << "\"}";

        rows.push_back(row.str());
    }

    cout << "{\"departure\":\"" << clockText(departure)
         << "\",\"allGreen\":" << (allGreen ? "true" : "false")
         << ",\"routeStatus\":\""
         << (allGreen
             ? "All configured signals are green at simulated arrival times."
             : "One or more signals are outside their configured green windows.")
         << "\",\"congruences\":\"" << equations
         << "\",\"crtPossible\":" << (possible ? "true" : "false")
         << ",";

    if (possible) {
        cout << "\"crtResidue\":" << a
             << ",\"crtModulus\":" << m
             << ",\"crtTime\":\"" << clockText(a)
             << "\",\"crtExplanation\":\"T ≡ "
             << a << " (mod " << m << ").\"";
    } else {
        cout << "\"crtResidue\":null"
             << ",\"crtModulus\":null"
             << ",\"crtTime\":null"
             << ",\"crtExplanation\":\"No common exact target phase exists.\"";
    }

    cout << ",\"signals\":[";

    for (int i = 0; i < (int)rows.size(); i++) {
        if (i > 0) {
            cout << ",";
        }
        cout << rows[i];
    }

    cout << "]}";

    return 0;
}
