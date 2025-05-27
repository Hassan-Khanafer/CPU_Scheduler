import csv

# This class will keep track of all the traces that we parse through
class Trace:
    def __init__(self, trace, pid, throughput, meanTAT, waitTimes):
        self.trace = trace              # int to identify which trace it is
        self.process_pids = pid         # holds the pids of the processes within
        self.throughput = throughput    # holds the throughput of the processes in same order as pids
        self.meanTAT = meanTAT          # holds the TAT similarly
        self.waitTimes = waitTimes      # holds the wait time similarly

    def get_csv_row(self):
        """Generates a single row for the CSV output."""
        return [
            self.trace,
            ", ".join(map(str, self.process_pids)),
            ", ".join(map(str, self.throughput)) if isinstance(self.throughput, list) else self.throughput,
            ", ".join(map(str, self.meanTAT)) if isinstance(self.meanTAT, list) else self.meanTAT,
            ", ".join(map(str, self.waitTimes)),
        ]

# Function to process a single file and return a Trace object
def process_file(filename, trace_number):
    with open(filename, 'r') as file:
        lines = file.readlines()

    processes = {}
    for item in lines:
        if not ("+" in item or "Time" in item):  # Skip filler lines
            elements = [el.strip() for el in item.split('|') if el.strip()]
            time = int(elements[0])
            processID = int(elements[1])
            oldState = elements[2]
            newState = elements[3]

            if processID not in processes:
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

            if newState == "TERMINATED":
                processes[processID]['completionTime'] = time

    traceCompletionTime = 0
    PIDs = []
    waitTimes = []
    totalTAT = 0

    for processID, details in processes.items():
        totalWaitTime = 0
        PIDs.append(processID)
        traceCompletionTime = max(traceCompletionTime, details['completionTime'])
        totalTAT += (details['completionTime'] - details['arrivalTime'])

        for i in range(1, len(details['stateChanges'])):
            if details['stateChanges'][i]['prevState'] == "READY" and details['stateChanges'][i]['newState'] == "RUNNING":
                totalWaitTime += details['stateChanges'][i]['timestamp'] - details['stateChanges'][i-1]['timestamp']
        waitTimes.append(totalWaitTime)

    meanTAT = totalTAT / len(processes)
    throughput = traceCompletionTime / len(processes)

    return Trace(trace_number, PIDs, throughput, meanTAT, waitTimes)

# Main logic to process all files and output to one CSV
def process_all_files(output_csv):
    traces = []
    # Process each file
    for i in range(1, 11):  # From execution_1.txt to execution_10.txt
        filename = f'execution_{i}.txt'
        trace = process_file(filename, i)
        traces.append(trace)

    # Write all traces to the CSV
    with open(output_csv, 'w', newline='') as file:
        writer = csv.writer(file)
        # Write header
        writer.writerow(["Trace", "Process PIDs", "Throughput", "Mean TAT", "Wait Times"])
        # Write trace data
        for trace in traces:
            writer.writerow(trace.get_csv_row())

# Call the main function
process_all_files('output.csv')