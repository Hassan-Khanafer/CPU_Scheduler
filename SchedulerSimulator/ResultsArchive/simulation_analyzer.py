'''
The following code reads through the output files produced from our input_data tests and analyzes the following: 
throughput, average turnaround time, and the wait time of each process
The code will produce a CSV with all the parameters mentioned above for each process.
'''
import csv

# This class will keep track of all the traces that we parse through
class Trace:
    def __init__(self, trace,  pid, throughput, meanTAT, waitTimes):
        self.trace = trace              # int to identify which trace it is
        self.process_pids = pid         # holds the pids of the processes within
        self.throughput = throughput    # holds the throughput of the processes in same order as pids
        self.meanTAT = meanTAT            # holds the TAT similarly
        self.waitTimes = waitTimes        # holds the wait time similarly
    """
    def exportToCSV(self, filename):
        with open('output.csv', 'a', newline = '') as file:
            writer = csv.writer(file)
            writer.writerow(["Input Data Trace: "+ str(self.trace)])
            writer.writerow([self.process_pids])
            writer.writerow([self.throughput])
            writer.writerow([self.meanTAT])
            writer.writerow(self.waitTimes)
            writer.writerow([''])
            writer.writerow([''])
    """
    def exportToCSV(self, filename):
        with open('output.csv', 'a', newline='') as file:
            writer = csv.writer(file)

            # Write the trace number
            writer.writerow([f"Input Data Trace: {self.trace}"])
            
            # Write process PIDs
            writer.writerow(["Process PIDs"] + self.process_pids)

            # Write throughput
            writer.writerow(["Throughput"] + (self.throughput if isinstance(self.throughput, list) else [self.throughput]))

            # Write mean turnaround times
            writer.writerow(["Mean TAT"] + (self.meanTAT if isinstance(self.meanTAT, list) else [self.meanTAT]))

            # Write wait times
            writer.writerow(["Wait Times"] + self.waitTimes)

            # Add empty rows for readability
            writer.writerow([])
            writer.writerow([])

    def exportToTXT(self, filename):
        with open('Trace_Logistics.txt', 'a') as file: #This writes out the contents found in the previous section to an Output text file
            file.write('Trace' + str(Trace) + ' CPU Usage Logistics:\n')
            file.write('Total Overhead Time = ' + str() + '\n')
            file.write('Total IO Time = ' + str() + '\n')
            file.write('Ratio of Overhead Time to IO Time: ' + str() + '\n')
            file.write('\n')

# This list will hold the final analysis calculations for each trace
List_Traces = []

# Process States
READY = "READY"
NEW = "NEW"
RUNNING = "RUNNING"
WAITING = "WAITING"
TERMINATED = "TERMINATED"

# 
file_name = 'execution_1.txt' # + str(TestNumber) + '.txt'
with open(file_name, 'r') as file:
    lines = file.readlines() #Puts every single line into an index

processes = {}

for item in lines:
    if((not "+" in item) and (not "Time" in item)):     # Check that the current line is not a filler line, i.e. "+---+"
        # Strip all whitespace and split the columns into elements of a list
        elements = [el.strip() for el in item.split('|')]
        elements = [el for el in elements if el]
        # Set temp vars
        time = int(elements[0])
        processID = int(elements[1])
        oldState = elements[2]
        newState = elements[3]

        # If new process, initialize it
        if(processID not in processes):
            processes[processID] = {
                'arrivalTime': time,
                'stateChanges': [],
                'completionTime': None
            }

        processes[processID]['stateChanges'].append({
            'timestamp': time,
            'prevState': oldState,
            'newState': newState
        })

        # If process is terminated, store its completion time
        if(newState == TERMINATED):
            processes[processID]['completionTime'] = time

traceCompletionTime = 0
PIDs = []
waitTimes = []
totalTAT = 0

for processID, details in processes.items():
    totalWaitTime = 0
    PIDs.append(processID)
    if(traceCompletionTime <= details['completionTime']):
        traceCompletionTime = details['completionTime']
    totalTAT += (details['completionTime'] - details['arrivalTime'])
    for i in range(1, len(details['stateChanges'])):
        if(details['stateChanges'][i]['prevState'] == READY and details['stateChanges'][i]['newState'] == RUNNING):
            totalWaitTime += details['stateChanges'][i]['timestamp']- details['stateChanges'][i-1]['timestamp']
    waitTimes.append(totalWaitTime)

meanTAT = totalTAT / len(processes)
throughput = traceCompletionTime / len(processes)

print(PIDs, throughput, meanTAT, waitTimes)

trace1 = Trace(1, PIDs, throughput, meanTAT, waitTimes)
trace1.exportToCSV("test")

'''

'''