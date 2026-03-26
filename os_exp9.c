#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_REQ 200

typedef struct {
	int movement;
	int finalHead;
	int finalDirection; /* 1 for larger, -1 for smaller */
	int order[MAX_REQ];
	int orderCount;
} Result;

static int cmpInt(const void *a, const void *b) {
	int x = *(const int *)a;
	int y = *(const int *)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}

static int absDiff(int a, int b) {
	return (a > b) ? (a - b) : (b - a);
}

static void appendOrder(Result *res, int track) {
	if (res->orderCount < MAX_REQ) {
		res->order[res->orderCount++] = track;
	}
}

static void printResult(const char *name, Result res) {
	int i;
	printf("\n%s Result:\n", name);
	printf("Service order: ");
	for (i = 0; i < res.orderCount; i++) {
		printf("%d", res.order[i]);
		if (i != res.orderCount - 1) {
			printf(" -> ");
		}
	}
	if (res.orderCount == 0) {
		printf("(none)");
	}
	printf("\nTotal head movement: %d\n", res.movement);
}

static Result runFCFS(const int req[], int n, int head) {
	int i;
	Result res = {0};
	int cur = head;

	for (i = 0; i < n; i++) {
		res.movement += absDiff(cur, req[i]);
		cur = req[i];
		appendOrder(&res, req[i]);
	}

	res.finalHead = cur;
	return res;
}

static Result runLIFO(const int req[], int n, int head) {
	int i;
	Result res = {0};
	int cur = head;

	for (i = n - 1; i >= 0; i--) {
		res.movement += absDiff(cur, req[i]);
		cur = req[i];
		appendOrder(&res, req[i]);
	}

	res.finalHead = cur;
	return res;
}

static Result runRSS(const int req[], int n, int head) {
	int i;
	Result res = {0};
	int cur = head;
	int temp[MAX_REQ];

	for (i = 0; i < n; i++) temp[i] = req[i];

	for (i = n - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int t = temp[i];
		temp[i] = temp[j];
		temp[j] = t;
	}

	for (i = 0; i < n; i++) {
		res.movement += absDiff(cur, temp[i]);
		cur = temp[i];
		appendOrder(&res, temp[i]);
	}

	res.finalHead = cur;
	return res;
}

static Result runSSTF(const int req[], int n, int head) {
	int i, done = 0;
	int visited[MAX_REQ] = {0};
	Result res = {0};
	int cur = head;

	while (done < n) {
		int bestIdx = -1;
		int bestDist = 1000000000;

		for (i = 0; i < n; i++) {
			if (!visited[i]) {
				int d = absDiff(cur, req[i]);
				if (d < bestDist) {
					bestDist = d;
					bestIdx = i;
				}
			}
		}

		if (bestIdx == -1) break;

		visited[bestIdx] = 1;
		res.movement += absDiff(cur, req[bestIdx]);
		cur = req[bestIdx];
		appendOrder(&res, req[bestIdx]);
		done++;
	}

	res.finalHead = cur;
	return res;
}

static Result runLOOKLike(const int req[], int n, int head, int direction, int circular, int includeEnds, int diskSize) {
	int i;
	int sorted[MAX_REQ];
	int split = 0;
	Result res = {0};
	int cur = head;

	for (i = 0; i < n; i++) sorted[i] = req[i];
	qsort(sorted, n, sizeof(int), cmpInt);

	while (split < n && sorted[split] < head) split++;

	if (direction == 1) {
		for (i = split; i < n; i++) {
			res.movement += absDiff(cur, sorted[i]);
			cur = sorted[i];
			appendOrder(&res, sorted[i]);
		}

		if (circular) {
			if (split > 0) {
				if (includeEnds) {
					if (cur != diskSize - 1) {
						res.movement += absDiff(cur, diskSize - 1);
						cur = diskSize - 1;
					}
					res.movement += absDiff(cur, 0);
					cur = 0;
				} else {
					if (cur != sorted[0]) {
						res.movement += absDiff(cur, sorted[0]);
						cur = sorted[0];
					}
					appendOrder(&res, sorted[0]);
					for (i = 1; i < split; i++) {
						res.movement += absDiff(cur, sorted[i]);
						cur = sorted[i];
						appendOrder(&res, sorted[i]);
					}
					res.finalHead = cur;
					res.finalDirection = 1;
					return res;
				}
				for (i = 0; i < split; i++) {
					res.movement += absDiff(cur, sorted[i]);
					cur = sorted[i];
					appendOrder(&res, sorted[i]);
				}
			}
			res.finalDirection = 1;
		} else {
			if (split > 0) {
				if (includeEnds && cur != diskSize - 1) {
					res.movement += absDiff(cur, diskSize - 1);
					cur = diskSize - 1;
				}
				for (i = split - 1; i >= 0; i--) {
					res.movement += absDiff(cur, sorted[i]);
					cur = sorted[i];
					appendOrder(&res, sorted[i]);
				}
			}
			res.finalDirection = -1;
		}
	} else {
		for (i = split - 1; i >= 0; i--) {
			res.movement += absDiff(cur, sorted[i]);
			cur = sorted[i];
			appendOrder(&res, sorted[i]);
		}

		if (circular) {
			if (split < n) {
				if (includeEnds) {
					if (cur != 0) {
						res.movement += absDiff(cur, 0);
						cur = 0;
					}
					res.movement += absDiff(cur, diskSize - 1);
					cur = diskSize - 1;
				} else {
					if (cur != sorted[n - 1]) {
						res.movement += absDiff(cur, sorted[n - 1]);
						cur = sorted[n - 1];
					}
					appendOrder(&res, sorted[n - 1]);
					for (i = n - 2; i >= split; i--) {
						res.movement += absDiff(cur, sorted[i]);
						cur = sorted[i];
						appendOrder(&res, sorted[i]);
					}
					res.finalHead = cur;
					res.finalDirection = -1;
					return res;
				}
				for (i = n - 1; i >= split; i--) {
					res.movement += absDiff(cur, sorted[i]);
					cur = sorted[i];
					appendOrder(&res, sorted[i]);
				}
			}
			res.finalDirection = -1;
		} else {
			if (split < n) {
				if (includeEnds && cur != 0) {
					res.movement += absDiff(cur, 0);
					cur = 0;
				}
				for (i = split; i < n; i++) {
					res.movement += absDiff(cur, sorted[i]);
					cur = sorted[i];
					appendOrder(&res, sorted[i]);
				}
			}
			res.finalDirection = 1;
		}
	}

	res.finalHead = cur;
	return res;
}

static Result runSCAN(const int req[], int n, int head, int direction, int diskSize) {
	return runLOOKLike(req, n, head, direction, 0, 1, diskSize);
}

static Result runCSCAN(const int req[], int n, int head, int direction, int diskSize) {
	return runLOOKLike(req, n, head, direction, 1, 1, diskSize);
}

static Result runLOOK(const int req[], int n, int head, int direction) {
	return runLOOKLike(req, n, head, direction, 0, 0, 0);
}

static Result runCLOOK(const int req[], int n, int head, int direction) {
	return runLOOKLike(req, n, head, direction, 1, 0, 0);
}

static Result runNStepSCAN(const int req[], int n, int head, int direction, int nStep) {
	int start = 0;
	Result total = {0};
	int curHead = head;
	int curDir = direction;

	while (start < n) {
		int size = nStep;
		int i;
		int chunk[MAX_REQ];
		Result one;

		if (start + size > n) size = n - start;
		for (i = 0; i < size; i++) chunk[i] = req[start + i];

		one = runLOOK(chunk, size, curHead, curDir);
		total.movement += one.movement;
		for (i = 0; i < one.orderCount; i++) appendOrder(&total, one.order[i]);
		curHead = one.finalHead;
		curDir = one.finalDirection;
		start += size;
	}

	total.finalHead = curHead;
	total.finalDirection = curDir;
	return total;
}

static Result runFSCAN(const int req[], int n, int head, int direction, int diskSize, int firstQueueSize) {
	int i;
	int q1[MAX_REQ], q2[MAX_REQ];
	int n1 = firstQueueSize;
	int n2 = n - firstQueueSize;
	Result r1, r2, total = {0};

	if (n1 < 0) n1 = 0;
	if (n1 > n) n1 = n;
	n2 = n - n1;

	for (i = 0; i < n1; i++) q1[i] = req[i];
	for (i = 0; i < n2; i++) q2[i] = req[n1 + i];

	r1 = runSCAN(q1, n1, head, direction, diskSize);
	total.movement += r1.movement;
	for (i = 0; i < r1.orderCount; i++) appendOrder(&total, r1.order[i]);

	r2 = runSCAN(q2, n2, r1.finalHead, r1.finalDirection, diskSize);
	total.movement += r2.movement;
	for (i = 0; i < r2.orderCount; i++) appendOrder(&total, r2.order[i]);

	total.finalHead = r2.finalHead;
	total.finalDirection = r2.finalDirection;
	return total;
}

static void printMenu(void) {
	printf("\n===== Disk Scheduling Algorithms =====\n");
	printf("1. FCFS\n");
	printf("2. SSTF\n");
	printf("3. SCAN\n");
	printf("4. C-SCAN\n");
	printf("5. LOOK\n");
	printf("6. C-LOOK\n");
	printf("7. RSS (Random Scheduling)\n");
	printf("8. LIFO\n");
	printf("9. N-STEP SCAN (LOOK style per chunk)\n");
	printf("10. F-SCAN (two fixed queues)\n");
	printf("11. Exit\n");
	printf("Choose an option: ");
}

int main(void) {
	int n, i;
	int req[MAX_REQ];
	int head;
	int diskSize;
	int choice;
	int direction;

	srand((unsigned int)time(NULL));

	printf("Disk Scheduling Experiment\n");
	printf("Enter total number of requests (max %d): ", MAX_REQ);
	if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_REQ) {
		printf("Invalid request count.\n");
		return 1;
	}

	printf("Enter request queue: ");
	for (i = 0; i < n; i++) {
		if (scanf("%d", &req[i]) != 1 || req[i] < 0) {
			printf("Invalid track number.\n");
			return 1;
		}
	}

	printf("Enter initial head position: ");
	if (scanf("%d", &head) != 1 || head < 0) {
		printf("Invalid head position.\n");
		return 1;
	}

	printf("Enter disk size (number of tracks): ");
	if (scanf("%d", &diskSize) != 1 || diskSize <= 0) {
		printf("Invalid disk size.\n");
		return 1;
	}

	if (head >= diskSize) {
		printf("Head position must be less than disk size.\n");
		return 1;
	}

	for (i = 0; i < n; i++) {
		if (req[i] >= diskSize) {
			printf("Request %d is out of disk range [0, %d].\n", req[i], diskSize - 1);
			return 1;
		}
	}

	while (1) {
		Result ans;
		printMenu();
		if (scanf("%d", &choice) != 1) {
			printf("Invalid input.\n");
			return 1;
		}

		if (choice == 11) {
			printf("Exiting program.\n");
			break;
		}

		switch (choice) {
			case 1:
				ans = runFCFS(req, n, head);
				printResult("FCFS", ans);
				break;
			case 2:
				ans = runSSTF(req, n, head);
				printResult("SSTF", ans);
				break;
			case 3:
				printf("Direction (1 for larger, -1 for smaller): ");
				scanf("%d", &direction);
				if (direction != 1 && direction != -1) {
					printf("Invalid direction.\n");
					break;
				}
				ans = runSCAN(req, n, head, direction, diskSize);
				printResult("SCAN", ans);
				break;
			case 4:
				printf("Direction (1 for larger, -1 for smaller): ");
				scanf("%d", &direction);
				if (direction != 1 && direction != -1) {
					printf("Invalid direction.\n");
					break;
				}
				ans = runCSCAN(req, n, head, direction, diskSize);
				printResult("C-SCAN", ans);
				break;
			case 5:
				printf("Direction (1 for larger, -1 for smaller): ");
				scanf("%d", &direction);
				if (direction != 1 && direction != -1) {
					printf("Invalid direction.\n");
					break;
				}
				ans = runLOOK(req, n, head, direction);
				printResult("LOOK", ans);
				break;
			case 6:
				printf("Direction (1 for larger, -1 for smaller): ");
				scanf("%d", &direction);
				if (direction != 1 && direction != -1) {
					printf("Invalid direction.\n");
					break;
				}
				ans = runCLOOK(req, n, head, direction);
				printResult("C-LOOK", ans);
				break;
			case 7:
				ans = runRSS(req, n, head);
				printResult("RSS", ans);
				break;
			case 8:
				ans = runLIFO(req, n, head);
				printResult("LIFO", ans);
				break;
			case 9: {
				int nStep;
				printf("Direction (1 for larger, -1 for smaller): ");
				scanf("%d", &direction);
				if (direction != 1 && direction != -1) {
					printf("Invalid direction.\n");
					break;
				}
				printf("Enter N for N-STEP SCAN: ");
				scanf("%d", &nStep);
				if (nStep <= 0) {
					printf("N must be positive.\n");
					break;
				}
				ans = runNStepSCAN(req, n, head, direction, nStep);
				printResult("N-STEP SCAN", ans);
				break;
			}
			case 10: {
				int firstQueueSize;
				printf("Direction (1 for larger, -1 for smaller): ");
				scanf("%d", &direction);
				if (direction != 1 && direction != -1) {
					printf("Invalid direction.\n");
					break;
				}
				printf("Enter size of first queue (0 to %d): ", n);
				scanf("%d", &firstQueueSize);
				if (firstQueueSize < 0 || firstQueueSize > n) {
					printf("Invalid first queue size.\n");
					break;
				}
				ans = runFSCAN(req, n, head, direction, diskSize, firstQueueSize);
				printResult("F-SCAN", ans);
				break;
			}
			default:
				printf("Invalid choice. Please try again.\n");
				break;
		}
	}

	return 0;
}
