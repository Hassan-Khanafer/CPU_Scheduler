#pragma once // Include guard to prevent multiple inclusion

#include <thread>
#include <semaphore.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <filesystem>
#include <sstream>

using namespace std;

const string DATABASEFILENAME = "database.txt";

// Struct representing a database entry
struct DataBaseCounter {             
    int studentID;                              // Partition number
    vector<int> counter;                        // till 15, should hold 5 elements
};

DataBaseCounter *DB;

void Marking(int key);
void unlockKeys(int key);
void lockKeys(int key);