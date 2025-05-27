#include "Part2_870_895.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <filesystem>
#include <iostream> 
#include <signal.h> 
#include <cstdlib>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

using namespace std;

// Holds each student in the database
vector<DataBaseCounter> database;
// Holds the TA output files
vector<ofstream> TAFiles(5);

// Holds the id of the semaphore
int key_sem;

// Read files: opens files and returns a vector that contains each line in the file
vector<DataBaseCounter> ReadFile(string path){
    vector<DataBaseCounter> table {};
    ifstream inputFile(path);

    if (!inputFile) {
        cerr << "Unable to open file" << endl;
        return table; // Exit if the file cannot be opened
    }

    //store each line in the vector table
    string line;
    while(getline(inputFile, line)) {
        table.push_back(DataBaseCounter{stoi(line), {0, 0, 0, 0, 0}});
    }

    inputFile.close();
    return table; //return vector containing lines of the file
}

// Create a random number generator with an upper and lower bound! (inclusive)
int randomNumberGenerator(int lower_bound, int upper_bound) {
    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> distr(lower_bound, upper_bound);

    return distr(gen);
}

void lockKeys(int key){
    cout << "attempting to lock " << key << endl;
    struct sembuf lock1 = {(short unsigned int) key, -1, IPC_CREAT};
    struct sembuf lock2 = {(short unsigned int) (key + 1 % 5), -1, IPC_CREAT};

    struct sembuf locks[2] = {lock1, lock2};

    while(semop(key_sem, locks, 2) < 0){
    }
}   

void unlockKeys(int key){
    struct sembuf unlock1 = {(short unsigned int) key, 1, IPC_CREAT};
    struct sembuf unlock2 = {(short unsigned int) (key + 1 % 5), 1, IPC_CREAT};

    struct sembuf unlocks[2] = {unlock1, unlock2};

    semop(key_sem, unlocks, 2);
}

void Marking(int key) {
    bool doneCounting = false;
    int count = 0;
    while(!doneCounting) {
        unsigned int index = count % database.size(); //Index to the next student
        lockKeys(key); //Attempt to grab 2 keys to be able to access the database
        // Critical section starts here
        DataBaseCounter &currentStudent = database[index]; //Access the database and grab the next student
        sleep(randomNumberGenerator(1,4)); //Accessing Database Overhead
        
        // Marking!
        if(currentStudent.counter[key] < 3) {
            int studentId = currentStudent.studentID;
            TAFiles[key] << "Student: " << studentId << ", Grade Received: " << randomNumberGenerator(1, 10) << endl;
            cout << "TA " << key + 1 << " is marking." << endl;
            sleep(randomNumberGenerator(1,3)); //Marking Overhead
            currentStudent.counter[key]+=1;
        }
        else{
            doneCounting = true;
        }
        // Critical section ends here
        unlockKeys(key);
        count++;
    }
}

int main() 
{
    //Sets Up Student Data Base
    database = ReadFile(filesystem::current_path().string() + "/" + "database.txt");

    //Creates TA1 file
    TAFiles[0].open(filesystem::current_path().string() + "/" + "TA1.txt");
    TAFiles[1].open(filesystem::current_path().string() + "/" + "TA2.txt");
    TAFiles[2].open(filesystem::current_path().string() + "/" + "TA3.txt");
    TAFiles[3].open(filesystem::current_path().string() + "/" + "TA4.txt");
    TAFiles[4].open(filesystem::current_path().string() + "/" + "TA5.txt");

    key_sem = semget(IPC_PRIVATE , 5, 0666);

    if(key_sem == -1) {
        perror("semget failed");
        exit(1);
    }

    semctl(key_sem, 0, SETVAL, 1);
    semctl(key_sem, 1, SETVAL, 1);
    semctl(key_sem, 2, SETVAL, 1);
    semctl(key_sem, 3, SETVAL, 1);
    semctl(key_sem, 4, SETVAL, 1);

    for (int key = 0; key  < 5; key ++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            // Handle error if fork fails
            perror("fork");
            exit(1);
        }
        // Child process block
        else if (pid == 0) {
            cout << "Hi, I am TA" << key+1 << endl;

            Marking(key);

            exit(0);  // End child process after task
        }
    }

    // Parent waits for all child processes to finish
    for (int i = 0; i < 5; i++) {
        wait(NULL);  // Wait for each child process
    }

    TAFiles[0].close();
    TAFiles[1].close();
    TAFiles[2].close();
    TAFiles[3].close();
    TAFiles[4].close();

    return 0;
}