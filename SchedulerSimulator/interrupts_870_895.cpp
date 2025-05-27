#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include "interrupts_870_895.hpp"

using namespace std;

// Global timer variable to store the current time throughout the trace in milliseconds
int globalTime = 0;
// Global timer for RR; will decrement everytime a running program advances in time, and reset when a new program is chosen
int globalRRTimer = 0;

// Set up a counter that will be decremented as programs finish
int numIncompletePrograms;
// Counts the number of programs assigned to reduce overhead in checking the input_data file
int programsAssigned;

// Global variables for the path of files
string currentPath;         // current working directory
string inputFilePath;       // path of the trace file
string executionFilePath;   // path for execution file with the name included
string memoryStatusFilePath;// path for memory status

// Contains the short term scheduling algorithm chosen
string CPUScheduler;

// DELETE THESE
vector<string> trace;
vector<string> vector_table;

//Defines vectors that will contain each line from the input files
vector<InputProgram> input_data;

//Defines global output files
ofstream execFile;
ofstream memoryStatusFile;

// Variables to keep track of the total memory space and memory space available
int totalMemory = 40 + 25 + 15 + 10 + 8 + 2;
int totalFreeMemory = totalMemory;
int UsableFreeMemory = totalMemory;

// Struct representing a partition of memory
struct MemPartition {             
    unsigned int partitionNumber;   // Partition number
    unsigned int size;              // Size of partition
    int Occupier;                   // -1 = Free space, else, we put the PID of whats occupying it
};

// Defines the top pointer to be null
PCB* currentProgram = nullptr;

// Defines arrays of structs which will be referred to as tables; 
// Special note for memory partitions: index of partition number (X) in the array will be (X - 1)
MemPartition MemoryPartitionsTable[6];
PCB PCBTable[6];

// Simulates the ready queue by maintaining order based on when a program joins it
// Each element will hold a reference to the PCB program associated with it
struct ReadyQueueElement{
    PCB* program;
    ReadyQueueElement* next;
};

//Global pointers (head/tail) to the ready queue. Also maintaining size of ready queue
ReadyQueueElement* head = nullptr;
ReadyQueueElement* tail = nullptr;
unsigned int sizeOfRQ = 0;

// Pushes a RQ element to the end/tail of the ready queue linked list
void PushRQ (PCB* pg){
    ReadyQueueElement* temp = new ReadyQueueElement{pg, nullptr};

    if(tail != nullptr){
        tail->next = temp;
    }
    else {
        head = temp;
    }

    tail = temp;
    sizeOfRQ++;
}

// Pops a specific RQ element based on PID from the ready queue linked list
// Traverse through linked list and remove element!
PCB* PopRQPID (unsigned int pid){
    if(head == nullptr) {
        return nullptr;
    }

    ReadyQueueElement* temp = nullptr;
    ReadyQueueElement* prevIter = nullptr;
    ReadyQueueElement* iterator = head;
    while (iterator != nullptr) {
        if (iterator->program->pid == pid){
            temp = iterator;
            if(iterator == head) {
                head = iterator->next;
            }
            else {
                prevIter->next = iterator->next;
            }
            if(iterator == tail) {
                tail = prevIter;
            }
            PCB* program = temp->program;
            sizeOfRQ--;
            delete temp;
            return program;
        }
        else {
            prevIter = iterator;
            iterator = iterator->next;
        }
    }
    return nullptr;
}

// Pops the head of the ready queue linked list, which is the first program to enter it
PCB* PopRQHead (){
    if(head == nullptr) {
        return nullptr;
    }

    ReadyQueueElement* temp = head;
    head = head->next;

    if(temp == tail) {
        tail = nullptr;
    }

    PCB* program = temp->program;
    sizeOfRQ--;
    delete temp;
    return program;
}

// Create a random number generator with an upper and lower bound! (inclusive)
int randomNumberGenerator(int lower_bound, int upper_bound) {
    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> distr(lower_bound, upper_bound);

    return distr(gen);
}

// Returns true if target is found in str, false if not
bool searchForIn(string target, string str) {
    return str.find(target) != string::npos;
}

// Read files: opens files and returns a vector that contains each line in the file
vector<InputProgram> ReadFile(string path){
    vector<InputProgram> table;
    ifstream inputFile(path);

    if (!inputFile) {
        cerr << "Unable to open file" << endl;
        return table; // Exit if the file cannot be opened
    }

    // store each line in the file
    string line;

    // Loop through each line in the input_data file
    while(getline(inputFile, line)) {
        // Get a vector that contains the numbers in the current line without ', '
        vector<int> numbers = ReadLine(line);
        //Converting all int to unsigned int due to compiling errors on MAC 
        vector<unsigned int> unsignedNumbers(numbers.size());
        transform(numbers.begin(), numbers.end(), unsignedNumbers.begin(),
                   [](int num) { return static_cast<unsigned int>(num); });

        if(unsignedNumbers.size() == 6) {           // Make sure that numbers was filled correctly by the ReadLine method
            InputProgram temp = {unsignedNumbers[0],unsignedNumbers[1],unsignedNumbers[2],unsignedNumbers[3],unsignedNumbers[4],unsignedNumbers[5], UNASSIGNED};
            table.push_back(temp);
        }
    }

    inputFile.close();
    return table; //return vector containing lines of the file
}

// Method to get all the program parameters from the line in the input_data file
vector<int> ReadLine(string line) {
    vector<int> numbers;
    stringstream ss(line);
    string temp;

    // Error checking to make sure the line has a ','
    if(!searchForIn(",", line)) {
        cerr << "Could not find , in input file" << endl;
        return numbers;
    }

    // Loop through the stringstream elements that are separated by ','
    while(getline(ss, temp, ',')){
        // Remove whitespace at the start and end
        temp.erase(0, temp.find_first_not_of(' '));
        temp.erase(temp.find_last_not_of(' ') + 1);
        // Add the number to the vector
        numbers.push_back(stoi(temp));
    }
    return numbers;
}

// Sets up global variables to contain the correct path based on trace number
void setPaths(int inputNumber){
    // Stores the current working directory
    currentPath = std::filesystem::current_path().string() + "/";
    inputFilePath = currentPath + INPUTFILENAME + to_string(inputNumber) + ".txt";
    executionFilePath = currentPath + EXECUTIONFILENAME;
    memoryStatusFilePath = currentPath + MEMORYSTATUSFILENAME;
}

// Sets up structs with initial values, output files with table headers, and other global variables
void INIT() {
    // Setting up Memory Partitions table
    for(unsigned int i = 0; i < 6; i++) {
        MemPartition temp = {i+1, DEFINED_MEMORY_PARTITIONS[i], -1};
        MemoryPartitionsTable[i] = temp;
    }

    // Set the number of programs that need to be completed
    numIncompletePrograms = input_data.size();
    programsAssigned = 0;

    // Set up output file table headers
    execFile << "+------------------------------------------------------+" << endl;
    execFile << "| Time of Transition | PID |  Old State  |  New State  |" << endl;
    execFile << "+------------------------------------------------------+" << endl;

    memoryStatusFile << "+---------------------------------------------------------------------------------------------------+" << endl;
    memoryStatusFile << "| Time of Event | Memory Used |      Partitions State      | Total Free Memory | Usable Free Memory |" << endl;
    memoryStatusFile << "+---------------------------------------------------------------------------------------------------+" << endl;
    printToMemoryFile();
}

// Prints scheduling information to execution file
void printToExecFile(unsigned int pid, string prevState, string newState) {
    execFile << "|        " << setw(12) << left << globalTime 
             << "|  " << setw(3) << left << pid 
             << "|   " << setw(10) << left << prevState 
             << "|  " << setw(11) << left << newState << "|" << endl;
}

// Prints current memory status to execution file
void printToMemoryFile () {
    string temp;
    memoryStatusFile << "|       " << setw(8) << left <<globalTime 
    << "|      " << setw(7) << left << totalMemory - totalFreeMemory;
    for(int i = 0; i < 6; i++) {
        temp += to_string(MemoryPartitionsTable[i].Occupier);
        if(i != 5) {
            temp += ", ";
        }
    }
    memoryStatusFile << "|   " << setw(25) << left << temp 
    << "|        " << setw(11) << left << totalFreeMemory 
    << "|        " << setw(12) << left << UsableFreeMemory << "|" << endl;
}

// Finds first unoccupied spot in PCB table and returns that index
int findFirstPCBSpace() {
    for(unsigned int i = 0; i < 6; i++) {
        if(PCBTable[i].state != RUNNING && PCBTable[i].state != WAITING && PCBTable[i].state != READY) {
            return i;
        }
    }
    return -1;
}

// Updates PCB table based on command: adds a new program or removes a completed program
PCB* updatePCB(PCB temp, string command) {
    if(command == ADD) {
        unsigned int available_space = findFirstPCBSpace();
        PCBTable[available_space] = temp;
        cout << "PCB table admits process with pid: " << PCBTable[available_space].pid << endl;
        return &PCBTable[available_space];
    }
}

// Finds smallest memory partition for our program
int findBestMemoryPart(int size) {
    int best = -1;
    for(unsigned int i = 0; i < 6; i++) {
        if(MemoryPartitionsTable[i].size >= size && MemoryPartitionsTable[i].Occupier == -1) {
            if(MemoryPartitionsTable[i].size <= MemoryPartitionsTable[best].size || best == -1) { 
                best = i;
            }
        }
    }
    return best;
}

// Update memory partition table based on if a program is being added or removed; then, prints to memory file
void updateMemoryPartition(int partitionNumber, unsigned int pid, unsigned int pgSize, string command) {
    if(command == ADD) {
        MemoryPartitionsTable[partitionNumber].Occupier = pid;
        totalFreeMemory -= pgSize;
        UsableFreeMemory -= MemoryPartitionsTable[partitionNumber].size;
    }
    else if (command == REMOVE) {
        MemoryPartitionsTable[partitionNumber].Occupier = -1;
        totalFreeMemory += pgSize;
        UsableFreeMemory += MemoryPartitionsTable[partitionNumber].size;
    }
    printToMemoryFile(); 
}

// Adds new programs to the ready queue
bool newPGToReadyQueue(InputProgram pg){
    unsigned int bestPartition = findBestMemoryPart(pg.memorySize);
    if(bestPartition != -1) {
        PCB temp = {pg.pid, READY, pg.memorySize, bestPartition, pg.IOFrequency, pg.IODuration, 
                            pg.totalCPUTime, pg.IODuration, pg.IOFrequency};
        updateMemoryPartition(bestPartition, temp.pid, temp.size, ADD);
        PushRQ(updatePCB(temp, ADD));
        return true;
    }
    else {
        cout << "Program: " << pg.pid << " could not be allocated in memory. It is waiting for another program to complete." << endl;
        return false;
        // Program will have to wait
    }
}

// Checks for programs that completed waiting and adds them back to the Ready Queue
void checkWaitingPrograms() {

    for(int i = 0; i < 6; i++) {
        if(PCBTable[i].state == WAITING && PCBTable[i].remainingIOTime == 0) {
            PushRQ(&PCBTable[i]);
            PCBTable[i].state = READY;
            printToExecFile(PCBTable[i].pid, WAITING, PCBTable[i].state);
        }
    }
}

// Checks which programs have arrived based on input file and adds it to the ready queue
void checkArrivals(){
    // Check if CPU even needs to check arrivals from input_data
    if(programsAssigned < input_data.size()){
        for(int i = 0; i < input_data.size(); i++) {
            // If global time has reached or passed arrival time of a process and it is not assigned, assign it
            if(input_data[i].arrivalTime <= globalTime && !input_data[i].isAssigned){
                if(newPGToReadyQueue(input_data[i])){
                    input_data[i].isAssigned = ASSIGNED;
                    programsAssigned++;
                    printToExecFile(input_data[i].pid, NEW, READY);
                }
                else {
                    // If process could not be assigned at this global time, it will be continuously checked until it is assigned
                }
            }
        }
    }
}

//This is to iterate through the PCB table and find the the highest priority program
unsigned int findPriority(){
    unsigned int smallest = PCBTable[0].pid;
    for (int i = 0; i <= 5; i++){
        //First 'if' takes care of when things have just arrived
        if (PCBTable[i].pid <= smallest && input_data[i].arrivalTime == globalTime && PCBTable[i].state == READY){
            smallest = PCBTable[i].pid;
            cout << "the global time is" << endl;
        }
        //Second 'if' takes care of when things are idle (i.e. nothing has arrived and processes are ready)
        else if ( PCBTable[i].pid <= smallest && PCBTable[i].state == READY)
        {
            smallest = PCBTable[i].pid;
        }    
    }
    return smallest;
}

// Selects next program to be executed based on FCFS algorithm and updates states of previous program and new program
// Selects next program to be executed based on FCFS algorithm and updates states of previous program and new program
void FCFS() {
    // For first program or after idle time
    if(currentProgram == nullptr) {
        if(sizeOfRQ > 0) {
            currentProgram = PopRQHead();
            currentProgram->state = RUNNING;
            printToExecFile(currentProgram->pid, READY, currentProgram->state);
        }
    }
    else {
        // For current program that needs to perform IO
        if(currentProgram->remainingCPUTime > 0 && currentProgram->timeTillIO == 0) {
            currentProgram->state = WAITING;
            currentProgram->remainingIOTime = currentProgram->IODuration;
            currentProgram->timeTillIO = currentProgram->IOFreq;
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);
            
            PCB* nextProgram = PopRQHead();
            currentProgram = nextProgram;

            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
            }
        }
        // For completed programs
        else if(currentProgram->remainingCPUTime == 0) {
            currentProgram->state = TERMINATED;
            numIncompletePrograms--;
            updateMemoryPartition(currentProgram->processPartition, currentProgram->pid, currentProgram->size, REMOVE);
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);

            PCB* nextProgram = PopRQHead();
            currentProgram = nextProgram;

            // If there are more programs to run, we move onward!
            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
            }
        }
    }
}
void EP(){
    //Sets up the first program to run or idle time
    if(currentProgram == nullptr) { //current program always initially null
            if(sizeOfRQ > 0) {
                currentProgram = PopRQPID(findPriority()); // Pops the the the lowest priority PID program to start running it
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);  
            }    
    }
    else {
    // For programs that are already running
        if(currentProgram->remainingCPUTime > 0 && currentProgram->timeTillIO == 0) {
            currentProgram->state = WAITING;
            currentProgram->remainingIOTime = currentProgram->IODuration;
            currentProgram->timeTillIO = currentProgram->IOFreq;
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);
            
            PCB* nextProgram = PopRQPID(findPriority());
            cout << nextProgram << endl;
            currentProgram = nextProgram;

            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                cout << currentProgram->pid << endl;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
                }
            }
    //For Completed Programs
        else if(currentProgram->remainingCPUTime == 0) {
            currentProgram->state = TERMINATED;
            numIncompletePrograms--;
            updateMemoryPartition(currentProgram->processPartition, currentProgram->pid, currentProgram->size, REMOVE);
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);

            PCB* nextProgram = PopRQPID(findPriority());
            currentProgram = nextProgram;

            // If there are more programs to run, we move onward!
            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
            }
        } 
    }
}
void RR() {
    // For first program or after idle time
    if(currentProgram == nullptr) {
        if(sizeOfRQ > 0) {
            currentProgram = PopRQHead();
            currentProgram->state = RUNNING;
            globalRRTimer = RRQUANTUM;
            printToExecFile(currentProgram->pid, READY, currentProgram->state);
        }
    }
    else {
        // For current program that needs to perform IO
        if(currentProgram->remainingCPUTime > 0 && currentProgram->timeTillIO == 0) {
            currentProgram->state = WAITING;
            currentProgram->remainingIOTime = currentProgram->IODuration;
            currentProgram->timeTillIO = currentProgram->IOFreq;
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);
            
            PCB* nextProgram = PopRQHead();
            currentProgram = nextProgram;
            globalRRTimer = RRQUANTUM;          // Reset global RR timer once new program is picked

            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
            }
        }
        // For current program that ran out of Quantum Time!
        else if(currentProgram->remainingCPUTime > 0 && globalRRTimer == 0) {
            currentProgram->state = READY;
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);
            PushRQ(currentProgram);     // Pushes the program that just ran out of Quantum time into the RQ
            
            PCB* nextProgram = PopRQHead();
            currentProgram = nextProgram;
            globalRRTimer = RRQUANTUM;          // Reset global RR timer once new program is picked

            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
            }
        }
        // For completed programs
        else if(currentProgram->remainingCPUTime == 0) {
            currentProgram->state = TERMINATED;
            numIncompletePrograms--;
            updateMemoryPartition(currentProgram->processPartition, currentProgram->pid, currentProgram->size, REMOVE);
            printToExecFile(currentProgram->pid, RUNNING, currentProgram->state);

            PCB* nextProgram = PopRQHead();
            currentProgram = nextProgram;
            globalRRTimer = RRQUANTUM;          // Reset global RR timer once new program is picked

            // If there are more programs to run, we move onward!
            if(currentProgram != nullptr) {
                currentProgram->state = RUNNING;
                printToExecFile(currentProgram->pid, READY, currentProgram->state);
            }
        }
    }
}



// This method advances the global time and decrements all necessary parameters such as remainingCPUTime and remainingIOTime
void advanceTime() {
    // Advance programs waiting for IO
    for(int i = 0; i < 6; i++) {
        if(PCBTable[i].state == WAITING && PCBTable[i].remainingIOTime > 0) {
            PCBTable[i].remainingIOTime--;
        }
    }
    cout << "we make it here atleast, right?" << endl;
    if(currentProgram != nullptr) {
        if(currentProgram->remainingCPUTime > 0 && currentProgram->state != WAITING) {
            currentProgram->remainingCPUTime--;
            currentProgram->timeTillIO--;
        }
        cout << currentProgram->remainingCPUTime << endl;
    }

    globalTime++;
}

// Function to extract the number from the string
int extractNumber(const std::string& filename) {
    size_t underscore_pos = filename.find_last_of('_');
    size_t dot_pos = filename.find_last_of('.');

    if (underscore_pos == std::string::npos || dot_pos == std::string::npos || dot_pos < underscore_pos) {
        throw std::invalid_argument("Invalid filename format.");
    }

    std::string number_str = filename.substr(underscore_pos + 1, dot_pos - underscore_pos - 1);
    return std::stoi(number_str);  // Convert the extracted string to an integer
}

int main(int argc, char* argv[])
{
    string input_test = argv[1]; 
    int TEST_NUMBER = extractNumber(input_test);

    // NEED TO UPDATE LATER TO TAKE argv[1]
    //Use argv so that TA can just run .exe files by switching the number of the trace file
    // 1 IS THE INPUT DATA NUMBER
    setPaths(TEST_NUMBER);

    CPUScheduler = argv[2]; // for now, we can change it to argv[2] later
    
    //Sets up a vector where each element represents a program that will enter our simulator
    input_data = ReadFile(inputFilePath);
    
    //Creates exectution file
    execFile.open(executionFilePath);
    
    //Creates memory status file
    memoryStatusFile.open(memoryStatusFilePath);
    
    // Check if the files were successfully created
    if (!execFile || !memoryStatusFile) {
        cerr << "Error creating file!" << endl;
        return 1;
    }

    INIT();         // Set up arrays of structs and initializes the PCB with the init program
    
    while(numIncompletePrograms > 0){
        checkWaitingPrograms();      // Checks which process finished I/O and which process has just arrived to the CPU
        checkArrivals();             // Updates ready queue at the start of a new unit of time and saves state changes to output files
        if(CPUScheduler == FCFSALGO) {
            FCFS();
        }
        else if(CPUScheduler == PRIORITYALGO) {
            EP();
        }
        else if(CPUScheduler == RRALGO) {
            RR();
        }
        else {
            cerr << "Could not select CPU scheduler" << endl;
        }
        
        if(numIncompletePrograms != 0) {
            advanceTime();
            
        } 
    } 

    execFile << "+------------------------------------------------------+" << endl;
    memoryStatusFile << "+---------------------------------------------------------------------------------------------------+" << endl;

    //close the output files!
    execFile.close();
    memoryStatusFile.close();
    
    return 0; 
}

// Handles the messages for CPU processes
void CPUPrint(string line) {
    int duration = stoi(line.substr(line.find(",") +2)); //Finds the argument of CPU command (i.e. how long the CPU will execute for)
    execFile << globalTime << ", " << duration << ", CPU execution" << endl; //Writes to the output file the CPU execution
    globalTime = globalTime + duration; //Add to the globalTime
}

// Handles SYSCALLS/Interrupts
void SYSCALL(string line) {
    //Step 1 of interrupt is the mode switch to kernel
    execFile << globalTime << ", " << KERNEL_SWITCH << ", switch to kernel mode" << endl;
    globalTime = globalTime + KERNEL_SWITCH;
    
    //Step 2 is saving the context 
    int context_time =  randomNumberGenerator(1,3);
    execFile << globalTime << ", " << context_time << ", context saved" << endl;
    globalTime = globalTime + context_time;
    
    //Step 3 is finding the appropriate ISR within the vector table
    int vectorPosition = stoi(line.substr(8,line.find(",") - 8));
    execFile << globalTime << ", " << VECTOR_SWITCH << ", find vector " << vectorPosition << " in memory position " << vector_table[vectorPosition] << endl;
    globalTime = globalTime + VECTOR_SWITCH;

    //Step 4 is to load the ISR found in Step 3 onto the CPU
    execFile << globalTime << ", " << LOAD_ISR_ADDRESS << ", load address " << vector_table[vectorPosition].substr(0,6);;
    execFile << " into the PC" << endl;
    globalTime = globalTime + LOAD_ISR_ADDRESS;

    //Step 5 is to actually run the ISR
    //Use RNG to get the times for each step of SYSCALL
    int total_time = stoi(line.substr(line.find(",") +2));
    int ISR_time = randomNumberGenerator(1, total_time - 2);
    int transfer_time = randomNumberGenerator(1, total_time - ISR_time - 1);
    int error_check_time = total_time - ISR_time - transfer_time;
    //Write the values and messages to the output file
    execFile << globalTime << ", " << ISR_time << ", SYSCALL: run the ISR" << endl;
    globalTime = globalTime + ISR_time;
    execFile << globalTime << ", " << transfer_time << ", transfer data" << endl;
    globalTime = globalTime + transfer_time;
    execFile << globalTime << ", " << error_check_time << ", check for errors" << endl;
    globalTime = globalTime + error_check_time;

    //Step 6 is to go back to the saved context (i.e. before the interrupt)
    execFile << globalTime << ", " << IRET_TIME << ", IRET" << endl;
    globalTime = globalTime + IRET_TIME;
}

// Handles all types of ENDIO processes
void ENDIOPrint(string line) {
    //Step 1 is to check priority of the interrupt
    execFile << globalTime << ", " << INTERRUPT_PRIORITY_CHECK << ", check priority of interrupt" << endl;
    globalTime = globalTime + INTERRUPT_PRIORITY_CHECK;

    //Step 2 is to check if the interrupt is masked
    execFile << globalTime << ", " << INTERRUPT_MASK << ", check if masked" << endl;
    globalTime = globalTime + INTERRUPT_MASK;

    //Step 3 is to switch to kernel mode
    execFile << globalTime << ", " << KERNEL_SWITCH << ", switch to kernel mode" << endl;
    globalTime = globalTime + KERNEL_SWITCH;

    //Step 4 is to save the current context of the program that was running
    int context_time =  randomNumberGenerator(1,3);
    execFile << globalTime << ", " << context_time << ", context saved" << endl;
    globalTime = globalTime + context_time;

    //Step 5 is finding the appropriate ISR in the vector table
    int vectorPosition = stoi(line.substr(7,line.find(",") - 7));
    execFile << globalTime << ", " << VECTOR_SWITCH << ", find vector " << vectorPosition << " in memory position " << vector_table[vectorPosition] << endl;
    globalTime = globalTime + VECTOR_SWITCH;

    //Step 6 is to load the address of the ISR onto the CPU
    execFile << globalTime << ", " << LOAD_ISR_ADDRESS << ", load address " << vector_table[vectorPosition].substr(0,6);
    execFile << " into the PC" << endl;
    globalTime = globalTime + LOAD_ISR_ADDRESS;
    
    //Step 7 is to run the end of interrupt process
    int end_io_time = stoi(line.substr(line.find(",") + 2));
    execFile << globalTime << ", " << end_io_time << ", END_IO " << endl;
    globalTime = globalTime + end_io_time;

    //Final step is to go back to the saved context (i.e. before the interrupt)
    execFile << globalTime << ", " << IRET_TIME << ", IRET" << endl;
    globalTime = globalTime + IRET_TIME;
}