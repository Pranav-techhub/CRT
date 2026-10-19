
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <numeric>
#include <cstdio>
using namespace std;

long long mod(long long a, long long m)
{
    return (a % m + m) % m;
}

long long extendedGCD(long long a, long long b,
                      long long &x, long long &y)
{
    if (b == 0)
    {
        x = 1;
        y = 0;
        return a;
    }

    long long x1, y1;
    long long g = extendedGCD(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return g;
}

bool combineCRT(long long &answer, long long &period,
                long long remainder, long long cycle,
                string &step)
{
    long long oldAnswer = answer;
    long long oldPeriod = period;
    long long g = gcd(oldPeriod, cycle);

    ostringstream out;

    out << "Combine T = " << oldAnswer
        << " (mod " << oldPeriod << ") with T = "
        << remainder << " (mod " << cycle << "). ";

    if ((remainder - answer) % g != 0)
    {
        out << "No common solution: the remainder difference "
            << "is not divisible by gcd " << g << ".";
        step = out.str();
        return false;
    }

    long long x, y;
    extendedGCD(oldPeriod / g, cycle / g, x, y);

    long long k = mod(
        ((remainder - answer) / g) * x,
        cycle / g
    );

    long long newPeriod = oldPeriod * (cycle / g);

    answer = mod(answer + oldPeriod * k, newPeriod);
    period = newPeriod;

    out << "gcd = " << g
        << ". Combined result: T = " << answer
        << " (mod " << period << ").";

    step = out.str();
    return true;
}

string clockText(long long seconds)
{
    seconds = mod(seconds, 86400);

    char output[16];

    snprintf(output, sizeof(output), "%02lld:%02lld:%02lld",
             seconds / 3600,
             (seconds % 3600) / 60,
             seconds % 60);

    return output;
}

string escapeJSON(const string &value)
{
    string output;

    for (char c : value)
    {
        if (c == '"' || c == '\\')
        {
            output += '\\';
            output += c;
        }
        else if (c == '\n')
        {
            output += "\\n";
        }
        else if (c == '\r')
        {
            output += "\\r";
        }
        else if (c == '\t')
        {
            output += "\\t";
        }
        else
        {
            output += c;
        }
    }

    return output;
}

struct Signal
{
    long long cycle;
    long long offset;
    long long greenStart;
    long long greenDuration;
    long long travel;
};

int main()
{
    string vehicle;
    long long departure;
    int n;

    if (!(cin >> vehicle >> departure >> n))
    {
        return 1;
    }

    if (n < 1 || n > 10)
    {
        cout << "{\"error\":\"Enter between 1 and 10 intersections.\"}";
        return 0;
    }

    vector<Signal> signals(n);

    for (int i = 0; i < n; i++)
    {
        cin >> signals[i].cycle
            >> signals[i].offset
            >> signals[i].greenStart
            >> signals[i].greenDuration
            >> signals[i].travel;

        if (signals[i].cycle <= 0 ||
            signals[i].greenDuration <= 0 ||
            signals[i].greenDuration > signals[i].cycle ||
            signals[i].travel < 0)
        {
            cout << "{\"error\":\"Invalid intersection settings.\"}";
            return 0;
        }
    }

    long long answer = 0;
    long long period = 1;
    long long elapsed = 0;

    bool possible = true;

    vector<string> equations;
    vector<string> steps;

    // Calculate one congruence for each intersection.
    for (int i = 0; i < n; i++)
    {
        Signal s = signals[i];

        long long targetPhase = mod(
            s.greenStart + (s.greenDuration - 1) / 2,
            s.cycle
        );

        // T + elapsed - offset = targetPhase (mod cycle)
        long long remainder = mod(
            s.offset + targetPhase - elapsed,
            s.cycle
        );

        ostringstream equation;
        equation << "T ≡ " << remainder
                 << " (mod " << s.cycle << ")";

        equations.push_back(equation.str());

        if (i == 0)
        {
            answer = remainder;
            period = s.cycle;

            ostringstream first;
            first << "Start: T ≡ " << answer
                  << " (mod " << period << ").";

            steps.push_back(first.str());
        }
        else if (possible)
        {
            string step;

            possible = combineCRT(
                answer, period, remainder, s.cycle, step
            );

            steps.push_back(step);
        }

        // Travel is from the previous point to this intersection.
        // The first intersection's travel value is ignored.
        if (i > 0)
        {
            elapsed += s.travel;
        }
    }

    long long targetDeparture = -1;

    if (possible)
    {
        // Find the first CRT solution at or after departure.
        targetDeparture = answer;

        if (targetDeparture < departure)
        {
            long long difference = departure - targetDeparture;
            long long jumps = difference / period;

            if (difference % period != 0)
            {
                jumps++;
            }

            targetDeparture += jumps * period;
        }
    }

    bool allGreen = possible;
    vector<string> signalRows;

    if (possible)
    {
        elapsed = 0;

        for (int i = 0; i < n; i++)
        {
            Signal s = signals[i];

            // The first intersection is reached at departure time.
            long long arrival = targetDeparture + elapsed;

            long long phase = mod(
                arrival - s.offset,
                s.cycle
            );

            long long greenDifference = mod(
                phase - s.greenStart,
                s.cycle
            );

            bool green = greenDifference < s.greenDuration;

            if (!green)
            {
                allGreen = false;
            }

            ostringstream row;

            row << "{\"name\":\"Intersection " << i + 1 << "\","
                << "\"arrival\":\"" << clockText(arrival) << "\","
                << "\"phase\":" << phase << ","
                << "\"cycle\":" << s.cycle << ","
                << "\"green\":" << (green ? "true" : "false") << ","
                << "\"status\":\"" << (green ? "Green" : "Red") << "\"}";

            signalRows.push_back(row.str());

            // Travel to the next intersection.
            elapsed += s.travel;
        }
    }

    cout << "{";

    cout << "\"vehicle\":\"" << escapeJSON(vehicle) << "\",";
    cout << "\"departure\":\"" << clockText(departure) << "\",";
    cout << "\"crtPossible\":" << (possible ? "true" : "false") << ",";

    cout << "\"congruences\":[";
    for (int i = 0; i < (int)equations.size(); i++)
    {
        if (i > 0) cout << ",";
        cout << "\"" << escapeJSON(equations[i]) << "\"";
    }
    cout << "],";

    cout << "\"crtSteps\":[";
    for (int i = 0; i < (int)steps.size(); i++)
    {
        if (i > 0) cout << ",";
        cout << "\"" << escapeJSON(steps[i]) << "\"";
    }
    cout << "],";

    if (possible)
    {
        cout << "\"crtResidue\":" << answer << ",";
        cout << "\"crtModulus\":" << period << ",";
        cout << "\"crtAbsoluteSeconds\":" << targetDeparture << ",";
        cout << "\"crtTime\":\"" << clockText(targetDeparture) << "\",";
        cout << "\"crtExplanation\":\"T = "
             << answer << " + " << period
             << "k seconds. The first solution at or after departure is "
             << clockText(targetDeparture) << ".\",";
    }
    else
    {
        cout << "\"crtResidue\":null,";
        cout << "\"crtModulus\":null,";
        cout << "\"crtAbsoluteSeconds\":null,";
        cout << "\"crtTime\":null,";
        cout << "\"crtExplanation\":\"No common solution exists for these congruences.\",";
    }

    cout << "\"signals\":[";
    for (int i = 0; i < (int)signalRows.size(); i++)
    {
        if (i > 0) cout << ",";
        cout << signalRows[i];
    }
    cout << "],";

    cout << "\"allGreen\":" << (allGreen ? "true" : "false") << ",";
    cout << "\"routeStatus\":\""
         << (allGreen
             ? "All signals green at calculated time"
             : (possible
                 ? "Some signals are red at calculated time"
                 : "No common CRT solution"))
         << "\"";

    cout << "}";

    return 0;
}
