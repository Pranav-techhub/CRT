#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Intersection {
    int cycle, offset, green_start, green_duration, travel_time;
};

int main(int argc, char* argv[]) {
    // Check if arguments are passed
    if(argc < 3) {
        cout << "{\"error\": \"Missing arguments\"}\n";
        return 1;
    }

    string vehicle = argv[1];
    int start_time = stoi(argv[2]);
    int priority = 3; // Default low priority

    // Auto-assign priority based on vehicle type
    if (vehicle == "Ambulance" || vehicle == "Fire_Brigade") priority = 1;
    else if (vehicle == "VIP_Convoy") priority = 2;

    // The search horizon changes based on priority. 
    // Higher priority vehicles search longer to find a guaranteed green corridor.
    int search_horizon = (priority == 1) ? 300 : ((priority == 2) ? 150 : 60);

    // Hardcoded intersections for demonstration
    vector<Intersection> route = {
        {60, 0,  0,  30, 15}, 
        {45, 10, 5,  20, 45}, 
        {90, 25, 20, 40, 80}  
    };

    int feasible_departure = -1;

    // Bounded search algorithm for CRT and Green-Window validation
    for (int d = start_time; d <= start_time + search_horizon; ++d) {
        bool passes_all = true;
        for (const auto& I : route) {
            int arrival_time = d + I.travel_time;
            int P = ((arrival_time - I.offset) % I.cycle + I.cycle) % I.cycle;
            int green_end = I.green_start + I.green_duration;
            
            bool isGreen = (green_end <= I.cycle) 
                ? (P >= I.green_start && P < green_end)
                : (P >= I.green_start || P < (green_end % I.cycle));

            if (!isGreen) {
                passes_all = false;
                break;
            }
        }
        if (passes_all) {
            feasible_departure = d;
            break;
        }
    }

    // Output JSON for the web server to read
    cout << "{";
    cout << "\"vehicle\": \"" << vehicle << "\", ";
    cout << "\"priority\": " << priority << ", ";
    
    if (feasible_departure != -1) {
        cout << "\"status\": \"success\", ";
        cout << "\"departure\": " << feasible_departure << ", ";
        cout << "\"wait_time\": " << (feasible_departure - start_time) << ", ";
        cout << "\"intersections\": [";
        for (size_t i = 0; i < route.size(); ++i) {
            const auto& I = route[i];
            int arrival = feasible_departure + I.travel_time;
            int phase = ((arrival - I.offset) % I.cycle + I.cycle) % I.cycle;
            cout << "{\"id\": " << i+1 << ", \"arrival\": " << arrival << ", \"phase\": " << phase << "}";
            if (i < route.size() - 1) cout << ", ";
        }
        cout << "]";
    } else {
        cout << "\"status\": \"failed\", ";
        cout << "\"message\": \"No feasible corridor found within horizon limit.\", ";
        cout << "\"horizon\": " << search_horizon;
    }
    cout << "}\n";

    return 0;
}
