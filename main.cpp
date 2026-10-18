#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Structure to hold intersection data
struct Intersection {
    int cycle;
    int offset;
    int green_start;
    int green_duration;
    int travel_time;
};

// Function to calculate if a given arrival time falls within the green window
bool isGreen(int arrival_time, const Intersection& I) {
    // Calculate current phase, ensuring positive modulo for time offsets
    int P = ((arrival_time - I.offset) % I.cycle + I.cycle) % I.cycle;
    
    int green_end = I.green_start + I.green_duration;
    
    // Handle standard green window
    if (green_end <= I.cycle) {
        return (P >= I.green_start && P < green_end);
    } 
    // Handle wrap-around green window (crosses the cycle boundary back to 0)
    else {
        return (P >= I.green_start || P < (green_end % I.cycle));
    }
}

int main() {
    cout << "========================================================\n";
    cout << "  Emergency Vehicle Priority Sync (CRT-Based Model)     \n";
    cout << "========================================================\n\n";

    // Inputs
    string vehicle_type = "Ambulance";
    int priority_level = 1; // 1 = Highest
    int earliest_start = 0;
    int search_horizon = 300; // Search up to 300 seconds

    // Pre-loaded demonstration data
    // {cycle, offset, green_start, green_duration, travel_time}
    vector<Intersection> route = {
        {60, 0,  0,  30, 15}, // Int 1: 60s cycle, 0 offset, green 0-30, travel time 15s
        {45, 10, 5,  20, 45}, // Int 2: 45s cycle, 10 offset, green 5-25, travel time 45s
        {90, 25, 20, 40, 80}  // Int 3: 90s cycle, 25 offset, green 20-60, travel time 80s
    };

    cout << "Vehicle: " << vehicle_type << " (Priority Level " << priority_level << ")\n";
    cout << "Requested Earliest Start Time: " << earliest_start << "s\n";
    cout << "Search Horizon: " << search_horizon << "s\n\n";

    int feasible_departure = -1;

    // Search algorithm to find the exact departure time
    for (int d = earliest_start; d <= earliest_start + search_horizon; ++d) {
        bool passes_all = true;
        
        for (const auto& I : route) {
            int arrival_time = d + I.travel_time;
            if (!isGreen(arrival_time, I)) {
                passes_all = false;
                break;
            }
        }
        
        if (passes_all) {
            feasible_departure = d;
            break;
        }
    }

    // Results Output
    if (feasible_departure != -1) {
        cout << ">>> SUCCESS: Earliest Feasible Departure Time is " << feasible_departure << "s <<<\n";
        cout << "Wait time: " << (feasible_departure - earliest_start) << "s\n\n";
        
        cout << "Intersection Arrival Details:\n";
        for (size_t i = 0; i < route.size(); ++i) {
            const auto& I = route[i];
            int arrival = feasible_departure + I.travel_time;
            int phase = ((arrival - I.offset) % I.cycle + I.cycle) % I.cycle;
            cout << "  Int " << i+1 << " | Arrival: " << arrival 
                 << "s | Phase at Arrival: " << phase 
                 << " | Status: GREEN [Pass]\n";
        }
    } else {
        cout << ">>> FAILED: No feasible departure found within horizon (" << search_horizon << "s). <<<\n";
        cout << "Try expanding the search horizon or checking intersection configurations.\n";
    }

    return 0;
}
