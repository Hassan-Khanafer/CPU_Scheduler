Team:
Hassan Khanafer 101196870
Fahim Haider 101186895

Please note the following about our submission:
- Please note that the last three digits of our student numbers were used for the naming of the input and output files to avoid 
lengthy file names. (HK - 870, FH - 895)

Scheduler Simulator:
    - It is assumed that the simulator does not receive a program larger than the biggest memory partition. For processes that 
    cannot find a spot in memory, they will continue to wait until a process completes and frees up a block.

    - This simulator prioritizes processes that that have entered the ready queue from WAITING state over processes that have just 
    arrived into the ready queue based on the input_data trace.

    - For FCFS, the simulator enters programs from the input_data into the ready queue in the same order that the programs are listed 
    in the file. This deviates from the PID-based solution on Brightspace.
    
    - For EP, the scheduler assigns the CPU based on the lowest PID. Lowest equal highest priority

    - For RR, it is assumed that the processes time till IO does not get 'refreshed' (set back to the IOFrequency) every time the 
    process gets preempted.

    - Test cases:
        1. FCFS: Identical test case to the example posted on Brightspace
        2. FCFS: 5 processes filling up 5 memory slots; P24 does not need IO!; All processes appear later that t = 0s.
        3. FCFS: 3 processes; P1 takes the only available memory slot for P20! P20 must wait until P1 completes.
        4. EP: 4 Processes with same arrival time but different PID
        5. EP: 3 processes with descending PID numbers come in sequentially to test if the scheduler can deal with dynamic process
        addition
        6. EP: 3 processes. The highest PID arrives first and has the shortest CPU time. However The latter two have higher priotity
        so the first process should experience some degree of starvation.
        7. EP: 3 processes. The third process arrives when the one of the two terminates and the other just re-entered the ready queue
        after an I/O process. To see if the scheduler still chooses the lowest PID despite late arrival 
        8. RR: Two programs compete for CPU time; Both processes have an IOFrequency > RR Quantum
        9. RR: Two programs always need to perform IO before being preempted by the scheduler
       10. RR: Six programs running to simulate how actual processors need to handle multiple long processes
    - To run the tests, please use the format './<.exe file from compilation> <inputdata file> <scheduler>
