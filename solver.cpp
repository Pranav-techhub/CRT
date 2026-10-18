
#include <iostream>
#include <string>
#include <cstdio>
using namespace std;

int main() {
    string vehicle;
    int departure, n;

    cin >> vehicle >> departure >> n;

    int cycle[8], offset[8], greenStart[8];
    int greenDuration[8], travel[8];

    int arrival[8], phase[8];
    bool green[8];

    int i, j;

    // Read signal information
    for (i = 0; i < n; i++) {
        cin >> cycle[i] >> offset[i] >> greenStart[i]
            >> greenDuration[i] >> travel[i];
    }

    int totalTravel = 0;
    bool allGreen = true;

    // Calculate arrival time and signal status
    for (i = 0; i < n; i++) {
        totalTravel = totalTravel + travel[i];

        arrival[i] = departure + totalTravel;

        phase[i] = (arrival[i] - offset[i]) % cycle[i];

        if (phase[i] < 0) {
            phase[i] = phase[i] + cycle[i];
        }

        int difference = phase[i] - greenStart[i];

        if (difference < 0) {
            difference = difference + cycle[i];
        }

        if (difference < greenDuration[i]) {
            green[i] = true;
        } else {
            green[i] = false;
            allGreen = false;
        }
    }

    // Find the target phase for each signal
    int target[8];
    int remainder[8];

    for (i = 0; i < n; i++) {
        target[i] = greenStart[i] + (greenDuration[i] - 1) / 2;

        if (target[i] >= cycle[i]) {
            target[i] = target[i] % cycle[i];
        }

        remainder[i] = (offset[i] + target[i] - 0) % cycle[i];

        remainder[i] = (remainder[i] - (totalTravel - travel[i])) % cycle[i];

        if (remainder[i] < 0) {
            remainder[i] = remainder[i] + cycle[i];
        }

        // Include travel to the current signal
        remainder[i] = (remainder[i] - travel[i]) % cycle[i];

        if (remainder[i] < 0) {
            remainder[i] = remainder[i] + cycle[i];
        }
    }

    // Calculate the least common multiple of cycle lengths
    int commonPeriod = 1;

    for (i = 0; i < n; i++) {
        int number = commonPeriod;
        int other = cycle[i];

        while (other != 0) {
            int temp = other;
            other = number % other;
            number = temp;
        }

        int gcd = number;
        commonPeriod = (commonPeriod / gcd) * cycle[i];
    }

    // Search for a departure time that matches every target phase
    int answer = -1;

    for (int time = 0; time < commonPeriod; time++) {
        bool match = true;

        for (i = 0; i < n; i++) {
            int signalArrival = time;

            for (j = 0; j <= i; j++) {
                signalArrival = signalArrival + travel[j];
            }

            int signalPhase = (signalArrival - offset[i]) % cycle[i];

            if (signalPhase < 0) {
                signalPhase = signalPhase + cycle[i];
            }

            if (signalPhase != target[i]) {
                match = false;
                break;
            }
        }

        if (match) {
            answer = time;
            break;
        }
    }

    // Print results in JSON format for the website
    int hour = (departure / 3600) % 24;
    int minute = (departure % 3600) / 60;

    char departureText[10];
    snprintf(departureText, sizeof(departureText),
             "%02d:%02d", hour, minute);

    cout << "{\"departure\":\"" << departureText << "\",";
    cout << "\"allGreen\":" << (allGreen ? "true" : "false") << ",";
    cout << "\"routeStatus\":\"";

    if (allGreen) {
        cout << "All configured signals are green at simulated arrival times.";
    } else {
        cout << "One or more signals are outside their configured green windows.";
    }

    cout << "\",\"congruences\":\"";

    for (i = 0; i < n; i++) {
        if (i > 0) {
            cout << "; ";
        }

        cout << "T ≡ " << remainder[i]
             << " (mod " << cycle[i] << ")";
    }

    cout << "\",\"crtPossible\":" << (answer != -1 ? "true" : "false");

    if (answer != -1) {
        int answerHour = (answer / 3600) % 24;
        int answerMinute = (answer % 3600) / 60;

        char answerText[10];
        snprintf(answerText, sizeof(answerText),
                 "%02d:%02d", answerHour, answerMinute);

        cout << ",\"crtResidue\":" << answer;
        cout << ",\"crtModulus\":" << commonPeriod;
        cout << ",\"crtTime\":\"" << answerText << "\"";
        cout << ",\"crtExplanation\":\"A matching departure time was found by checking possible times one by one.\"";
    } else {
        cout << ",\"crtResidue\":null";
        cout << ",\"crtModulus\":" << commonPeriod;
        cout << ",\"crtTime\":null";
        cout << ",\"crtExplanation\":\"No matching target departure time was found in one complete common period.\"";
    }

    cout << ",\"signals\":[";

    for (i = 0; i < n; i++) {
        if (i > 0) {
            cout << ",";
        }

        int arrivalHour = (arrival[i] / 3600) % 24;
        int arrivalMinute = (arrival[i] % 3600) / 60;

        char arrivalText[10];
        snprintf(arrivalText, sizeof(arrivalText),
                 "%02d:%02d", arrivalHour, arrivalMinute);

        cout << "{\"name\":\"Intersection " << i + 1 << "\",";
        cout << "\"arrival\":\"" << arrivalText << "\",";
        cout << "\"phase\":" << phase[i] << ",";
        cout << "\"cycle\":" << cycle[i] << ",";
        cout << "\"green\":" << (green[i] ? "true" : "false") << ",";
        cout << "\"status\":\""
             << (green[i] ? "Green window" : "Outside green window")
             << "\"}";
    }

    cout << "]}";

    return 0;
}
