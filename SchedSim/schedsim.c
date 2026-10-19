#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "util.h"

// Comparator function for Priority Scheduling
int my_comparer(const void *this, const void *that) {
    const ProcessType *p1 = (const ProcessType *)this;
    const ProcessType *p2 = (const ProcessType *)that;

    if (p1->pri != p2->pri) {
        return p1->pri - p2->pri;
    }
    return p1->art - p2->art;
}

// Function to calculate turnaround time
void findTurnAroundTime(ProcessType plist[], int n) {
    for (int i = 0; i < n; i++) {
        plist[i].tat = plist[i].bt + plist[i].wt;
    }
}

// Robust FCFS handling arrival times and CPU idle periods
void findWaitingTimeFCFS(ProcessType plist[], int n) {
    int current_time = 0;
    for (int i = 0; i < n; i++) {
        if (current_time < plist[i].art) {
            current_time = plist[i].art; // Handle CPU idle time
        }
        plist[i].wt = current_time - plist[i].art;
        if (plist[i].wt < 0) plist[i].wt = 0;
        current_time += plist[i].bt;
    }
}

void findavgTimeFCFS(ProcessType plist[], int n) {
    findWaitingTimeFCFS(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n======= FCFS Implementation =======\n");
    printMetrics(plist, n);
}

// Priority Scheduling (sorting by priority then arrival)
void findavgTimePriority(ProcessType plist[], int n) {
    qsort(plist, n, sizeof(ProcessType), my_comparer);
    findWaitingTimeFCFS(plist, n); // Reuse FCFS logic after priority sorting
    findTurnAroundTime(plist, n);
    printf("\n======= Priority Scheduling =======\n");
    printMetrics(plist, n);
}

// Preemptive SJF / SRTF Waiting Time
void findWaitingTimeSJF(ProcessType plist[], int n) {
    int rem_bt[n];
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }

    int complete = 0, t = 0;
    int minm;
    int shortest = 0;
    int check = 0;

    while (complete != n) {
        minm = 9999;
        check = 0;

        for (int j = 0; j < n; j++) {
            if ((plist[j].art <= t) && (rem_bt[j] < minm) && (rem_bt[j] > 0)) {
                minm = rem_bt[j];
                shortest = j;
                check = 1;
            }
        }

        if (check == 0) {
            t++;
            continue;
        }

        rem_bt[shortest]--;

        if (rem_bt[shortest] == 0) {
            complete++;
            int finish_time = t + 1;
            plist[shortest].wt = finish_time - plist[shortest].art - plist[shortest].bt;
            if (plist[shortest].wt < 0) {
                plist[shortest].wt = 0;
            }
        }
        t++;
    }
}

void findavgTimeSJF(ProcessType plist[], int n) {
    findWaitingTimeSJF(plist, n);
    findTurnAroundTime(plist, n);
    printf("\n======= Shortest Job First (SJF) =======\n");
    printMetrics(plist, n);
}

// Round Robin (RR) handling arrival times properly
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) {
    int rem_bt[n];
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int t = 0;
    int completed = 0;

    // Find the earliest arrival time to start clock
    int min_art = plist[0].art;
    for (int i = 1; i < n; i++) {
        if (plist[i].art < min_art) {
            min_art = plist[i].art;
        }
    }
    if (t < min_art) t = min_art;

    while (completed < n) {
        int done_in_cycle = 1;

        for (int i = 0; i < n; i++) {
            // Process only if it has arrived and still has remaining burst time
            if (plist[i].art <= t && rem_bt[i] > 0) {
                done_in_cycle = 0;

                if (rem_bt[i] > quantum) {
                    t += quantum;
                    rem_bt[i] -= quantum;
                } else {
                    t += rem_bt[i];
                    plist[i].wt = t - plist[i].art - plist[i].bt;
                    if (plist[i].wt < 0) plist[i].wt = 0;
                    rem_bt[i] = 0;
                    completed++;
                }
            }
        }

        // If CPU is idle because no process has arrived yet, advance time
        if (done_in_cycle) {
            t++;
        }
    }
}

void findavgTimeRR(ProcessType plist[], int n, int quantum) {
    findWaitingTimeRR(plist, n, quantum);
    findTurnAroundTime(plist, n);
    printf("\n======= Round Robin (Quantum = %d) =======\n", quantum);
    printMetrics(plist, n);
}

int main(int argc, char *argv[]) {
    ParsedArgs succedeed = parse_args(argc, argv);
    if (!succedeed.valid) {
        return 1;
    }

    int n;
    ProcessType *plist = init_processes(succedeed.filename, &n);

    // Run FCFS
    ProcessType *plist_fcfs = clone_processes(plist, n);
    findavgTimeFCFS(plist_fcfs, n);
    free(plist_fcfs);

    // Run Priority
    ProcessType *plist_priority = clone_processes(plist, n);
    findavgTimePriority(plist_priority, n);
    free(plist_priority);

    // Run SJF
    ProcessType *plist_sjf = clone_processes(plist, n);
    findavgTimeSJF(plist_sjf, n);
    free(plist_sjf);

    // Run Round Robin
    ProcessType *plist_rr = clone_processes(plist, n);
    findavgTimeRR(plist_rr, n, succedeed.quantum);
    free(plist_rr);

    free(plist);
    return 0;
}