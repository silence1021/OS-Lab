#include <iostream>
#include <algorithm>
#include <deque>

using namespace std;

// Define a structure to represent a process
struct Process
{
    int arrival_time; // The time at which the process arrives
    int burst_time;   // The amount of time required by the process to complete
    int start_time;   // The time at which the process starts execution
    int finish_time;  // The time at which the process finishes execution
};

// Define an array of structures to store the arrival and burst times for each process
Process processes[] = {
    {1, 20},
    {5, 5},
    {18, 36},
    {20, 1},
    {25, 5},
    {26, 26},
    {27, 3},
    {28, 3},
    {30, 110},
    {35, 12},
    {50, 75},
    {55, 7},
    {100, 24},
    {101, 42},
    {112, 16},
    {120, 60},
    {200, 8},
    {205, 25},
    {220, 35},
    {225, 5},
    {235, 35},
    {240, 40},
    {250, 50},
    {300, 5},
    {305, 5},
    {310, 5},
    {315, 25}};

// Define a function to find the index of the process with minimum service time in the ready queue
int find_min_time_process_index(deque<Process> readyProcess)
{
    int n = readyProcess.size();
    int min_service_time = readyProcess[0].burst_time;
    int min_service_time_process_index = 0;
    for (int i = 1; i < n; i++)
    {
        if (readyProcess[i].burst_time < min_service_time)
        {
            min_service_time = readyProcess[i].burst_time;
            min_service_time_process_index = i;
        }
    }
    return min_service_time_process_index;
}

// Define a function to simulate the FIFO scheduling algorithm
void fifo_scheduling()
{
    int n = sizeof(processes) / sizeof(processes[0]);

    // Sort the processes in increasing order of their arrival time
    sort(processes, processes + n, [](const Process &p1, const Process &p2)
         { return p1.arrival_time < p2.arrival_time; });

    // Initialize the start time and finish time for each process
    int start_time[n];
    int finish_time[n];

    // Initialize the start time and finish time for the first process
    start_time[0] = processes[0].arrival_time;
    finish_time[0] = start_time[0] + processes[0].burst_time;

    // Process each process in order of their arrival time
    for (int i = 1; i < n; i++)
    {
        // Calculate the start time and finish time for the current process
        start_time[i] = max(finish_time[i - 1], processes[i].arrival_time);
        finish_time[i] = start_time[i] + processes[i].burst_time;
    }

    // Print the start time, finish time, and turn-around time for each process
    for (int i = 0; i < n; i++)
    {
        int turn_around_time = finish_time[i] - processes[i].arrival_time;
        int normalized_turn_around_time = turn_around_time / processes[i].burst_time;
        cout << "arrival time: " << processes[i].arrival_time << ", ";
        cout << "burst time: " << processes[i].burst_time << ", ";
        cout << "start time: " << start_time[i] << ", ";
        cout << "finish time: " << finish_time[i] << ", ";
        cout << "normalized turn-around time: " << normalized_turn_around_time << endl;
    }
}

// Define a function to simulate the SPN scheduling algorithm
// Define a function to simulate the SPN scheduling algorithm
void spn_scheduling()
{
    int n = sizeof(processes) / sizeof(processes[0]);

    // Sort the processes in increasing order of their arrival time
    sort(processes, processes + n, [](const Process &p1, const Process &p2)
         { return p1.arrival_time < p2.arrival_time; });

    deque<Process> newProcess;
    deque<Process> readyProcess;
    deque<Process> exitProcess;

    // Add the processes to the new process queue
    for (int i = 0; i < n; i++)
    {
        Process p;
        p.arrival_time = processes[i].arrival_time;
        p.burst_time = processes[i].burst_time;
        newProcess.push_back(p);
    }

    unsigned int current_time = 1; // current time of OS
    Process runningProcess;        // is running process
    while (!(newProcess.empty() && readyProcess.empty()))
    {
        while (true) // add arrived processes to ready queue
        {
            if (!newProcess.empty() && newProcess.front().arrival_time <= current_time)
            {
                runningProcess = newProcess.front(); // pointer to find ready process
                newProcess.pop_front();
                readyProcess.push_back(runningProcess);
            }
            else if (!readyProcess.empty() && readyProcess.front().arrival_time <= current_time)
            {
                break;
            }
            else
            {
                current_time++;
            }
        }

        // find min service time process and run
        if (!readyProcess.empty())
        {
            int min_service_time_process_index = find_min_time_process_index(readyProcess); // get min service time process index in ready process
            runningProcess = readyProcess.at(min_service_time_process_index);               // get running process (min service time process)
            current_time += runningProcess.burst_time;                                      // run
            runningProcess.finish_time = current_time;                                      // finish
            readyProcess.erase(readyProcess.begin() + min_service_time_process_index);      // delete running process from ready queue
            exitProcess.push_back(runningProcess);                                          // add running process into exit queue
        }
    }

    // print result
    for (int i = 0; i < n; i++)
    {
        int turn_around_time = exitProcess[i].finish_time - exitProcess[i].arrival_time;
        int normalized_turn_around_time = turn_around_time / exitProcess[i].burst_time;
        int start_time = exitProcess[i].finish_time - exitProcess[i].burst_time; // calculate start time
        cout << "arrival time: " << exitProcess[i].arrival_time << ", ";
        cout << "burst time: " << exitProcess[i].burst_time << ", ";
        cout << "start time: " << start_time << ", "; // print start time
        cout << "finish time: " << exitProcess[i].finish_time << ", ";
        cout << "normalized turn-around time: " << normalized_turn_around_time << endl;
    }
}

int main()
{
    cout << "FIFO scheduling:" << endl;
    fifo_scheduling();
    cout << endl;
    cout << "SPN scheduling:" << endl;
    spn_scheduling();
    return 0;
}