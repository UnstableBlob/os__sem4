#include<stdio.h>
#include<stdlib.h>
#include<limits.h>

#define MAX_PROCESSES 10

// Process structure
typedef struct {
    int pid;                // Process ID
    int arrival_time;       // Arrival time
    int burst_time;         // Total burst time
    int remaining_time;     // Remaining burst time (for preemptive)
    int priority;           // Priority (lower number = higher priority)
    int completion_time;    // Completion time
    int turnaround_time;    // Turnaround time
    int waiting_time;       // Waiting time
    int response_time;      // Response time
    int first_executed;     // Flag to track first execution
} Process;

// Function prototypes
void input_processes(Process processes[], int *n);
void reset_processes(Process original[], Process processes[], int n);
void srtn_scheduling(Process processes[], int n);
void round_robin_scheduling(Process processes[], int n, int quantum);
void priority_scheduling(Process processes[], int n);
void display_results(Process processes[], int n, char* algorithm);
void calculate_avg_times(Process processes[], int n);

int main() {
    Process processes[MAX_PROCESSES];
    Process temp[MAX_PROCESSES];
    int n, choice, quantum;
    
    printf("=== PREEMPTIVE SCHEDULING SIMULATOR ===\n\n");
    
    input_processes(processes, &n);
    
    while(1) {
        printf("\n\n=== MENU ===\n");
        printf("1. SRTN (Shortest Remaining Time Next)\n");
        printf("2. Round Robin\n");
        printf("3. Priority Scheduling (Preemptive)\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                reset_processes(processes, temp, n);
                srtn_scheduling(temp, n);
                display_results(temp, n, "SRTN");
                break;
                
            case 2:
                printf("Enter time quantum: ");
                scanf("%d", &quantum);
                reset_processes(processes, temp, n);
                round_robin_scheduling(temp, n, quantum);
                display_results(temp, n, "Round Robin");
                break;
                
            case 3:
                reset_processes(processes, temp, n);
                priority_scheduling(temp, n);
                display_results(temp, n, "Priority (Preemptive)");
                break;
                
            case 4:
                printf("Exiting...\n");
                exit(0);
                
            default:
                printf("Invalid choice!\n");
        }
    }
    
    return 0;
}

// Input process details
void input_processes(Process processes[], int *n) {
    printf("Enter number of processes: ");
    scanf("%d", n);
    
    for(int i = 0; i < *n; i++) {
        processes[i].pid = i + 1;
        printf("\nProcess %d:\n", i + 1);
        printf("  Arrival Time: ");
        scanf("%d", &processes[i].arrival_time);
        printf("  Burst Time: ");
        scanf("%d", &processes[i].burst_time);
        printf("  Priority (lower = higher priority): ");
        scanf("%d", &processes[i].priority);
        
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = -1;
        processes[i].first_executed = 0;
    }
}

// Reset processes for new scheduling run
void reset_processes(Process original[], Process processes[], int n) {
    for(int i = 0; i < n; i++) {
        processes[i] = original[i];
        processes[i].remaining_time = original[i].burst_time;
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = -1;
        processes[i].first_executed = 0;
    }
}

// SRTN (Shortest Remaining Time Next) Scheduling
void srtn_scheduling(Process processes[], int n) {
    int timer = 0;
    int completed = 0;
    int current_process = -1;
    
    printf("\n=== SRTN Execution Timeline ===\n");
    printf("Time\tProcess\n");
    
    while(completed < n) {
        int shortest = -1;
        int min_remaining = INT_MAX;
        
        // Find process with shortest remaining time that has arrived
        for(int i = 0; i < n; i++) {
            if(processes[i].arrival_time <= timer && 
               processes[i].remaining_time > 0 && 
               processes[i].remaining_time < min_remaining) {
                min_remaining = processes[i].remaining_time;
                shortest = i;
            }
        }
        
        if(shortest == -1) {
            // No process available, CPU idle
            printf("%d\tIDLE\n", timer);
            timer++;
            continue;
        }
        
        // Record response time on first execution
        if(!processes[shortest].first_executed) {
            processes[shortest].response_time = timer - processes[shortest].arrival_time;
            processes[shortest].first_executed = 1;
        }
        
        // Execute process for 1 time unit
        if(current_process != shortest) {
            printf("%d\tP%d\n", timer, processes[shortest].pid);
            current_process = shortest;
        }
        
        processes[shortest].remaining_time--;
        timer++;
        
        // Process completed
        if(processes[shortest].remaining_time == 0) {
            completed++;
            processes[shortest].completion_time = timer;
            processes[shortest].turnaround_time = processes[shortest].completion_time - processes[shortest].arrival_time;
            processes[shortest].waiting_time = processes[shortest].turnaround_time - processes[shortest].burst_time;
        }
    }
}

// Round Robin Scheduling
void round_robin_scheduling(Process processes[], int n, int quantum) {
    int timer = 0;
    int completed = 0;
    int queue[MAX_PROCESSES * 100];
    int front = 0, rear = 0;
    int in_queue[MAX_PROCESSES] = {0};
    
    printf("\n=== Round Robin Execution Timeline (Quantum = %d) ===\n", quantum);
    printf("Time\tProcess\n");
    
    // Add processes that arrive at time 0
    for(int i = 0; i < n; i++) {
        if(processes[i].arrival_time == 0) {
            queue[rear++] = i;
            in_queue[i] = 1;
        }
    }
    
    while(completed < n) {
        if(front == rear) {
            // Queue empty, advance time
            printf("%d\tIDLE\n", timer);
            timer++;
            
            // Add newly arrived processes
            for(int i = 0; i < n; i++) {
                if(processes[i].arrival_time == timer && !in_queue[i] && processes[i].remaining_time > 0) {
                    queue[rear++] = i;
                    in_queue[i] = 1;
                }
            }
            continue;
        }
        
        int current = queue[front++];
        in_queue[current] = 0;
        
        // Record response time
        if(!processes[current].first_executed) {
            processes[current].response_time = timer - processes[current].arrival_time;
            processes[current].first_executed = 1;
        }
        
        printf("%d\tP%d\n", timer, processes[current].pid);
        
        // Execute for quantum or remaining time, whichever is smaller
        int exec_time = (processes[current].remaining_time < quantum) ? 
                        processes[current].remaining_time : quantum;
        
        processes[current].remaining_time -= exec_time;
        timer += exec_time;
        
        // Add newly arrived processes during execution
        for(int i = 0; i < n; i++) {
            if(processes[i].arrival_time <= timer && 
               processes[i].arrival_time > timer - exec_time &&
               !in_queue[i] && processes[i].remaining_time > 0) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
        
        // Check if process completed
        if(processes[current].remaining_time == 0) {
            completed++;
            processes[current].completion_time = timer;
            processes[current].turnaround_time = processes[current].completion_time - processes[current].arrival_time;
            processes[current].waiting_time = processes[current].turnaround_time - processes[current].burst_time;
        } else {
            // Add back to queue
            queue[rear++] = current;
            in_queue[current] = 1;
        }
    }
}

// Priority Scheduling (Preemptive)
void priority_scheduling(Process processes[], int n) {
    int timer = 0;
    int completed = 0;
    int current_process = -1;
    
    printf("\n=== Priority Scheduling Execution Timeline ===\n");
    printf("Time\tProcess\n");
    
    while(completed < n) {
        int highest_priority = -1;
        int min_priority = INT_MAX;
        
        // Find process with highest priority (lowest priority number) that has arrived
        for(int i = 0; i < n; i++) {
            if(processes[i].arrival_time <= timer && 
               processes[i].remaining_time > 0 && 
               processes[i].priority < min_priority) {
                min_priority = processes[i].priority;
                highest_priority = i;
            }
        }
        
        if(highest_priority == -1) {
            // No process available, CPU idle
            printf("%d\tIDLE\n", timer);
            timer++;
            continue;
        }
        
        // Record response time on first execution
        if(!processes[highest_priority].first_executed) {
            processes[highest_priority].response_time = timer - processes[highest_priority].arrival_time;
            processes[highest_priority].first_executed = 1;
        }
        
        // Execute process for 1 time unit
        if(current_process != highest_priority) {
            printf("%d\tP%d (Priority: %d)\n", timer, processes[highest_priority].pid, processes[highest_priority].priority);
            current_process = highest_priority;
        }
        
        processes[highest_priority].remaining_time--;
        timer++;
        
        // Process completed
        if(processes[highest_priority].remaining_time == 0) {
            completed++;
            processes[highest_priority].completion_time = timer;
            processes[highest_priority].turnaround_time = processes[highest_priority].completion_time - processes[highest_priority].arrival_time;
            processes[highest_priority].waiting_time = processes[highest_priority].turnaround_time - processes[highest_priority].burst_time;
        }
    }
}

// Display scheduling results
void display_results(Process processes[], int n, char* algorithm) {
    printf("\n\n=== %s SCHEDULING RESULTS ===\n", algorithm);
    printf("┌─────┬─────────┬───────────┬───────────────┬───────────────┬──────────────┬──────────────┐\n");
    printf("│ PID │ Arrival │ Burst Time│ Completion    │ Turnaround    │ Waiting Time │ Response Time│\n");
    printf("├─────┼─────────┼───────────┼───────────────┼───────────────┼──────────────┼──────────────┤\n");
    
    for(int i = 0; i < n; i++) {
        printf("│ P%-2d │   %-5d │    %-6d │      %-8d │      %-8d │     %-8d │     %-9d│\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].response_time);
    }
    
    printf("└─────┴─────────┴───────────┴───────────────┴───────────────┴──────────────┴──────────────┘\n");
    
    calculate_avg_times(processes, n);
}

// Calculate and display average times
void calculate_avg_times(Process processes[], int n) {
    float avg_turnaround = 0, avg_waiting = 0, avg_response = 0;
    
    for(int i = 0; i < n; i++) {
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
        avg_response += processes[i].response_time;
    }
    
    avg_turnaround /= n;
    avg_waiting /= n;
    avg_response /= n;
    
    printf("\nAverage Turnaround Time: %.2f\n", avg_turnaround);
    printf("Average Waiting Time: %.2f\n", avg_waiting);
    printf("Average Response Time: %.2f\n", avg_response);
}