#include <iostream>
#include <vector>

using namespace std;

const int P = 5; 
const int R = 3; 

void calculateNeed(int need[P][R], int maxm[P][R], int allot[P][R]) {
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            need[i][j] = maxm[i][j] - allot[i][j];
}

bool isSafe(int avail[R], int maxm[P][R], int allot[P][R], int need[P][R], bool silent = false) {
    int work[R];
    for (int i = 0; i < R; i++)
        work[i] = avail[i];

    bool finish[P] = {0};
    vector<int> safeSeq;

    int count = 0;
    while (count < P) {
        bool found = false;
        for (int p = 0; p < P; p++) {
            if (finish[p] == 0) {
                int j;
                for (j = 0; j < R; j++)
                    if (need[p][j] > work[j])
                        break;

                if (j == R) {
                    for (int k = 0; k < R; k++)
                        work[k] += allot[p][k];

                    safeSeq.push_back(p);
                    finish[p] = 1;
                    found = true;
                    count++;
                }
            }
        }

        if (found == false) {
            if (!silent) cout << "System is in an UNSAFE state.\n";
            return false;
        }
    }

    if (!silent) {
        cout << "System is in a SAFE state.\nSafe Sequence is: ";
        for (int i = 0; i < P; i++) {
            cout << "P" << safeSeq[i] << (i != P - 1 ? " -> " : "");
        }
        cout << "\n";
    }
    return true;
}

void requestResource(int pid, int req[R], int avail[R], int maxm[P][R], int allot[P][R], int need[P][R]) {
    cout << "\n--- Process P" << pid << " requesting resources: ";
    for (int i = 0; i < R; i++) cout << req[i] << " ";
    cout << "---\n";

    for (int i = 0; i < R; i++) {
        if (req[i] > need[pid][i]) {
            cout << "Request REJECTED: Process exceeded its maximum claim.\n";
            return;
        }
    }

    for (int i = 0; i < R; i++) {
        if (req[i] > avail[i]) {
            cout << "Request REJECTED: Resources not currently available. P" << pid << " must wait.\n";
            return;
        }
    }

    for (int i = 0; i < R; i++) {
        avail[i] -= req[i];
        allot[pid][i] += req[i];
        need[pid][i] -= req[i];
    }

    if (isSafe(avail, maxm, allot, need, true)) {
        cout << "Request ACCEPTED: Allocation leaves system in a safe state.\n";
    } else {
        cout << "Request REJECTED: Allocation would lead to an UNSAFE state. Rolling back.\n";
        for (int i = 0; i < R; i++) {
            avail[i] += req[i];
            allot[pid][i] -= req[i];
            need[pid][i] += req[i];
        }
    }
}

int main() {
    int avail[R] = {3, 3, 2}; 

    int maxm[P][R] = {{7, 5, 3}, 
                      {3, 2, 2}, 
                      {9, 0, 2}, 
                      {2, 2, 2}, 
                      {4, 3, 3}};

    int allot[P][R] = {{0, 1, 0}, 
                       {2, 0, 0}, 
                       {3, 0, 2}, 
                       {2, 1, 1}, 
                       {0, 0, 2}};

    int need[P][R];
    calculateNeed(need, maxm, allot);

    cout << "INITIAL STATE CHECK:\n";
    isSafe(avail, maxm, allot, need);

    int req1[R] = {1, 0, 2};
    requestResource(1, req1, avail, maxm, allot, need);

    int req2[R] = {3, 3, 0}; 
    int req3[R] = {2, 0, 0}; 
    requestResource(0, req3, avail, maxm, allot, need);

    int req4[R] = {0, 4, 0};
    requestResource(4, req4, avail, maxm, allot, need);

    return 0;
}