#include "elevator_core.hpp"

struct Elevator {
    int id;
    int floor = 1;
    int sectorStart, sectorEnd;
    Dir direction = IDLE;
    enum State { UP_TO_TARGET, DOWN_COLLECT } state = UP_TO_TARGET;
    ArrayStack<Passenger> passengers;
    int doorState = 0; // 0=closed, 1=opening, 2=open, 3=closing
    int doorTimer = 0;
    int targetFloor = 1;
    int moveTimer = 0;
    
    Elevator(int id, int start, int end) : id(id), sectorStart(start), sectorEnd(end) {}
    
    bool isFull() const { return passengers.size() >= 15; }
    bool isEmpty() const { return passengers.empty(); }
    bool isAtFloor(int f) const { return floor == f; }
    bool inSector(int f) const { return f >= sectorStart && f <= sectorEnd; }
};

struct Result {
    vector<double> p95Total, avgWaitPerFloor;
    double overallAvgWait = 0, overallP95 = 0;
};

class ElevatorSimulation {
private:
    int K, seed;
    bool verbose;
    mt19937 rng;
    vector<Elevator> elevators;
    vector<vector<Passenger>> floorQueues; // floorQueues[f] = passengers waiting at floor f
    vector<Passenger> allPassengers;
    vector<Passenger> completedPassengers; // Track passengers who have completed their journey
    int currentTick = 0;
    
    // Constants for passenger generation
    static const int START_TIME = 6 * 3600 * 10; // 06:30:00 in ticks
    static const int END_TIME = 7 * 3600 * 10;   // 07:00:00 in ticks
    static const int PEAK_TIME = 6 * 3600 * 10 + 15 * 60 * 10; // 06:45:00 in ticks
    static const int APARTMENTS_PER_FLOOR = 8;
    static const int RESIDENTS_PER_APARTMENT = 3;
    static const int TOTAL_FLOORS = 30;
    
public:
    ElevatorSimulation(int K, int seed = 2025, bool verbose = false) 
        : K(K), seed(seed), verbose(verbose), rng(seed) {
        floorQueues.resize(TOTAL_FLOORS + 1);
        initializeElevators();
        generatePassengers();
    }
    
    void initializeElevators() {
        elevators.clear();
        if (K == 1) {
            elevators.emplace_back(1, 1, 30);
        } else if (K == 2) {
            elevators.emplace_back(1, 1, 15);
            elevators.emplace_back(2, 16, 30);
        } else if (K == 3) {
            elevators.emplace_back(1, 1, 10);
            elevators.emplace_back(2, 11, 20);
            elevators.emplace_back(3, 21, 30);
        }
    }
    
    void generatePassengers() {
        allPassengers.clear();
        int passengerId = 1;
        
        for (int floor = 2; floor <= TOTAL_FLOORS; floor++) {
            int residentsThisFloor = APARTMENTS_PER_FLOOR * RESIDENTS_PER_APARTMENT;
            
            for (int i = 0; i < residentsThisFloor; i++) {
                Passenger p;
                p.id = passengerId++;
                p.from = floor;
                p.to = 1;
                p.bornTick = generateBornTime();
                p.waitStartTick = p.bornTick;
                
                allPassengers.push_back(p);
            }
        }
        
        // Sort by born time for deterministic simulation
        sort(allPassengers.begin(), allPassengers.end(), 
             [](const Passenger& a, const Passenger& b) { return a.bornTick < b.bornTick; });
    }
    
    int generateBornTime() {
        // Triangular distribution with mode at 06:45
        uniform_real_distribution<double> dist(0.0, 1.0);
        double u = dist(rng);
        
        int mode = PEAK_TIME;
        int start = START_TIME;
        int end = END_TIME;
        
        int time;
        if (u < (double)(mode - start) / (end - start)) {
            // Left side of triangle
            time = start + sqrt(u * (mode - start) * (end - start));
        } else {
            // Right side of triangle
            time = end - sqrt((1 - u) * (end - mode) * (end - start));
        }
        
        return static_cast<int>(time);
    }
    
    void spawnPassengers() {
        for (const auto& p : allPassengers) {
            if (p.bornTick == currentTick) {
                Passenger newP = p; // Copy to avoid const issues
                floorQueues[p.from].push_back(newP);
                if (verbose) {
                    cout << fmt_time_hhmmss(currentTick, 6, 30, 0) 
                         << " Passenger " << p.id << " spawned at floor " << p.from << endl;
                }
            }
        }
    }
    
    void updateElevators() {
        for (auto& elevator : elevators) {
            updateElevator(elevator);
        }
    }
    
    void updateElevator(Elevator& elevator) {
        // Handle door operations
        if (elevator.doorState > 0) {
            elevator.doorTimer--;
            if (elevator.doorTimer <= 0) {
                if (elevator.doorState == 1) { // opening
                    elevator.doorState = 2; // open
                    if (verbose) {
                        cout << fmt_time_hhmmss(currentTick, 6, 30, 0) 
                             << " Elevator " << elevator.id << " doors opened at floor " << elevator.floor << endl;
                    }
                } else if (elevator.doorState == 3) { // closing
                    elevator.doorState = 0; // closed
                    if (verbose) {
                        cout << fmt_time_hhmmss(currentTick, 6, 30, 0) 
                             << " Elevator " << elevator.id << " doors closed at floor " << elevator.floor << endl;
                    }
                }
            }
        }
        
        // Handle movement
        if (elevator.moveTimer > 0) {
            elevator.moveTimer--;
            if (elevator.moveTimer == 0) {
                elevator.floor += elevator.direction;
                // Ensure floor bounds
                elevator.floor = max(1, min(30, elevator.floor));
                
                if (verbose) {
                    cout << fmt_time_hhmmss(currentTick, 6, 30, 0) 
                         << " Elevator " << elevator.id << " arrived at floor " << elevator.floor << endl;
                }
            }
        }
        
        // State machine logic
        if (elevator.doorState == 0 && elevator.moveTimer == 0) {
            if (elevator.state == Elevator::UP_TO_TARGET) {
                handleUpToTarget(elevator);
            } else if (elevator.state == Elevator::DOWN_COLLECT) {
                handleDownCollect(elevator);
            }
        }
        
        // Handle boarding/alighting when doors are open
        if (elevator.doorState == 2) {
            handleBoardingAlighting(elevator);
        }
    }
    
    void handleUpToTarget(Elevator& elevator) {
        // For down-peak, elevator should go to the highest floor with passengers in sector
        // and then collect all passengers on the way down
        int targetFloor = -1;
        for (int floor = elevator.sectorEnd; floor >= elevator.sectorStart; floor--) {
            if (!floorQueues[floor].empty()) {
                targetFloor = floor;
                break;
            }
        }
        
        if (targetFloor == -1) {
            // No passengers waiting, stay at current position
            return;
        }
        
        if (elevator.floor == targetFloor) {
            // Already at target, start collecting
            elevator.state = Elevator::DOWN_COLLECT;
            elevator.direction = DOWN;
            openDoors(elevator);
        } else {
            // Move towards target
            elevator.direction = (targetFloor > elevator.floor) ? UP : DOWN;
            elevator.moveTimer = T_MOVE_ONE;
        }
    }
    
    void handleDownCollect(Elevator& elevator) {
        if (elevator.floor == 1) {
            // At lobby, unload all passengers
            if (!elevator.isEmpty()) {
                openDoors(elevator);
            } else {
                // Empty, switch back to UP_TO_TARGET
                elevator.state = Elevator::UP_TO_TARGET;
                elevator.direction = IDLE;
            }
        } else {
            // Check if we should stop at current floor
            bool shouldStop = !floorQueues[elevator.floor].empty() && !elevator.isFull();
            
            if (shouldStop) {
                openDoors(elevator);
            } else {
                // Continue down - always move down one floor at a time
                elevator.moveTimer = T_MOVE_ONE;
            }
        }
    }
    
    void openDoors(Elevator& elevator) {
        elevator.doorState = 1; // opening
        elevator.doorTimer = T_OPEN;
    }
    
    void handleBoardingAlighting(Elevator& elevator) {
        // Unload passengers at floor 1
        if (elevator.floor == 1) {
            while (!elevator.isEmpty()) {
                Passenger p = elevator.passengers.pop();
                p.arriveTick = currentTick;
                completedPassengers.push_back(p); // Add to completed list
                if (verbose) {
                    cout << fmt_time_hhmmss(currentTick, 6, 30, 0) 
                         << " Passenger " << p.id << " arrived at lobby" << endl;
                }
            }
        }
        
        // Board passengers if not full and not at lobby
        if (elevator.floor != 1 && !elevator.isFull()) {
            auto& queue = floorQueues[elevator.floor];
            while (!queue.empty() && !elevator.isFull()) {
                Passenger p = queue.back();
                queue.pop_back();
                p.boardTick = currentTick;
                elevator.passengers.push(p);
                if (verbose) {
                    cout << fmt_time_hhmmss(currentTick, 6, 30, 0) 
                         << " Passenger " << p.id << " boarded elevator " << elevator.id << " at floor " << elevator.floor << endl;
                }
            }
        }
        
        // Close doors after a delay
        if (elevator.doorState == 2) {
            elevator.doorState = 3; // closing
            elevator.doorTimer = T_CLOSE;
        }
    }
    
    bool isSimulationComplete() {
        // Check if all passengers have been spawned and processed
        bool allPassengersSpawned = true;
        for (const auto& p : allPassengers) {
            if (p.bornTick > currentTick) {
                allPassengersSpawned = false;
                break;
            }
        }
        
        if (!allPassengersSpawned) {
            return false; // Still have passengers to spawn
        }
        
        // Check if there are any passengers still waiting on floors
        bool hasWaitingPassengers = false;
        for (int floor = 2; floor <= TOTAL_FLOORS; floor++) {
            if (!floorQueues[floor].empty()) {
                hasWaitingPassengers = true;
                break;
            }
        }
        
        // Check if any elevators still have passengers
        bool hasElevatorPassengers = false;
        for (const auto& elevator : elevators) {
            if (!elevator.isEmpty()) {
                hasElevatorPassengers = true;
                break;
            }
        }
        
        return !hasWaitingPassengers && !hasElevatorPassengers;
    }
    
    Result runSimulation() {
        currentTick = START_TIME;
        int maxTicks = END_TIME + 10 * 60 * 10; // 10 minutes after 07:00
        
        while (currentTick < maxTicks && !isSimulationComplete()) {
            spawnPassengers();
            updateElevators();
            currentTick++;
        }
        
        return calculateResults();
    }
    
    Result calculateResults() {
        Result result;
        result.p95Total.resize(TOTAL_FLOORS + 1, 0);
        result.avgWaitPerFloor.resize(TOTAL_FLOORS + 1, 0);
        
        // Calculate per-floor statistics
        vector<vector<int>> waitingTimesPerFloor(TOTAL_FLOORS + 1);
        vector<vector<int>> totalTimesPerFloor(TOTAL_FLOORS + 1);
        
        // Process completed passengers
        for (const auto& p : completedPassengers) {
            if (p.arriveTick != -1 && p.boardTick != -1) {
                int waitingTime = p.boardTick - p.waitStartTick;
                int totalTime = p.arriveTick - p.waitStartTick;
                
                waitingTimesPerFloor[p.from].push_back(waitingTime);
                totalTimesPerFloor[p.from].push_back(totalTime);
            }
        }
        
        vector<int> allWaitingTimes, allTotalTimes;
        
        for (int floor = 2; floor <= TOTAL_FLOORS; floor++) {
            if (!waitingTimesPerFloor[floor].empty()) {
                // Calculate average waiting time
                double sum = 0;
                for (int time : waitingTimesPerFloor[floor]) {
                    sum += time;
                }
                result.avgWaitPerFloor[floor] = sum / waitingTimesPerFloor[floor].size();
                
                // Calculate P95 total time
                sort(totalTimesPerFloor[floor].begin(), totalTimesPerFloor[floor].end());
                int p95Index = static_cast<int>(0.95 * totalTimesPerFloor[floor].size());
                if (p95Index >= totalTimesPerFloor[floor].size()) p95Index = totalTimesPerFloor[floor].size() - 1;
                result.p95Total[floor] = totalTimesPerFloor[floor][p95Index];
                
                allWaitingTimes.insert(allWaitingTimes.end(), waitingTimesPerFloor[floor].begin(), waitingTimesPerFloor[floor].end());
                allTotalTimes.insert(allTotalTimes.end(), totalTimesPerFloor[floor].begin(), totalTimesPerFloor[floor].end());
            } else {
                // No passengers completed from this floor - set a reasonable default
                result.p95Total[floor] = 3600 * 10; // 1 hour default
            }
        }
        
        // Calculate overall statistics
        if (!allWaitingTimes.empty()) {
            double sum = 0;
            for (int time : allWaitingTimes) {
                sum += time;
            }
            result.overallAvgWait = sum / allWaitingTimes.size();
        }
        
        if (!allTotalTimes.empty()) {
            sort(allTotalTimes.begin(), allTotalTimes.end());
            int p95Index = static_cast<int>(0.95 * allTotalTimes.size());
            if (p95Index >= allTotalTimes.size()) p95Index = allTotalTimes.size() - 1;
            result.overallP95 = allTotalTimes[p95Index];
        }
        
        return result;
    }
};

Result simulate_down_peak(int K, int seed = 2025, bool verbose = false) {
    ElevatorSimulation sim(K, seed, verbose);
    return sim.runSimulation();
}

int main() {
    cout << fixed << setprecision(1);
    
    for (int K = 1; K <= 3; K++) {
        cout << "\n=== K = " << K << " Elevators ===" << endl;
        Result result = simulate_down_peak(K, 2025, false);
        
        cout << "Average waiting time: " << result.overallAvgWait / 10.0 << " seconds" << endl;
        cout << "95th percentile total time: " << result.overallP95 / 10.0 << " seconds" << endl;
        
        // Per-floor P95 recommendations
        cout << "\nFloor: ";
        for (int floor = 2; floor <= 30; floor++) {
            cout << setw(6) << floor;
        }
        cout << endl;
        
        cout << "P95:   ";
        for (int floor = 2; floor <= 30; floor++) {
            cout << setw(6) << static_cast<int>(result.p95Total[floor] / 10.0);
        }
        cout << endl;
    }
    
    return 0;
}
