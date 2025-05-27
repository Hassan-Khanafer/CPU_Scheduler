#pragma once // Include guard to prevent multiple inclusion

#include <vector>
#include <string>

using namespace std;

// The name of the output files to be created
const string INPUTFILENAME = "input_data_";
const string EXECUTIONFILENAME = "execution_870_895.txt";
const string MEMORYSTATUSFILENAME = "memory_status_870_895.txt";

// Constant variables for each of the constant processes
const int KERNEL_SWITCH = 1;
const int VECTOR_SWITCH = 1;
const int IRET_TIME = 1;
const int LOAD_ISR_ADDRESS = 1;
const int INTERRUPT_PRIORITY_CHECK = 1;
const int INTERRUPT_MASK = 1;
const int FORK_VECTOR = 2;
const int EXEC_VECTOR = 3;

// The defined memory partitions: 40, 25, 15, 10, 8, 2
unsigned int DEFINED_MEMORY_PARTITIONS[6] = {40, 25, 15, 10, 8, 2};

// Constant strings representing state of PCB entries
const string NEW = "NEW";
const string READY = "READY";
const string RUNNING = "RUNNING";
const string WAITING = "WAITING";
const string TERMINATED = "TERMINATED";

// Strings to be used as commands for methods in .cpp file
const string ADD = "ADD";
const string REMOVE = "REMOVE";

// Constant strings representing the CPU Scheduling algorithm
const string FCFSALGO = "FCFS";
const string PRIORITYALGO = "EP";
const string RRALGO = "RR";

// Boolean variables to make it easy to determine if a program has been assigned or not
const bool ASSIGNED = true;
const bool UNASSIGNED = false;

// Constant that defines the quantum of the RR scheduling algorithm
const int RRQUANTUM = 100;

// Struct representing an entry in the PCB table
struct PCB {             
    unsigned int pid;               // PID number of process
    string state;                   // Process state: running, ready, waiting, terminated
    unsigned int size;              // Size of the process
    unsigned int processPartition;  // Partition Number that process is stored in
    unsigned int IOFreq;            // Frequency of I/O operation
    unsigned int IODuration;        // Duration of I/O operation
    unsigned int remainingCPUTime;  // Remaining CPU time
    unsigned int remainingIOTime;   // Remaining IO time: holds IODuration at initialization
    unsigned int timeTillIO;        // Time until next IO request: holds IOFreq at initialization
};

// Struct representing a program received in the input data file
struct InputProgram {
    unsigned int pid;
    unsigned int memorySize;
    unsigned int arrivalTime;
    unsigned int totalCPUTime;
    unsigned int IOFrequency;
    unsigned int IODuration;
    bool isAssigned;
};

void PushRQ (PCB* pg);
PCB* PopRQPID (unsigned int pid);
PCB* PopRQHead ();
int randomNumberGenerator(int lower_bound, int upper_bound);
bool searchForIn(string target, string str);
vector<InputProgram> ReadFile(string path);
vector<int> ReadLine (string line);
void setPaths();
void INIT();
void printToExecFile(unsigned int pid, string prevState, string newState);
void printToMemoryFile ();
int findFirstPCBSpace();
PCB* updatePCB(PCB temp, string command);
int findBestMemoryPart(int size);
void updateMemoryPartition(int partitionNumber, unsigned int pid, unsigned int pgSize, string command);
bool newPGToReadyQueue(InputProgram pg);
void checkWaitingPrograms();
void checkArrivals();
void FCFS();
void advanceTime();
void CPUPrint(string line);
void SYSCALL(string line);
void ENDIOPrint(string line);