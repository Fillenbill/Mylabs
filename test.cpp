#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

#define MAX_THREADS 100

HANDLE hMutex;  // Mutex for shared resources
HANDLE hEvent;  // Event for signaling when a thread finishes
int solutionCount = 0;  // Global variable to store the number of solutions

// Structure to pass data to each thread
typedef struct {
    int* coefficients;
    int n;
    int s;
} ThreadData;

// Function to evaluate the expression and check if it satisfies the equality
int evaluateExpression(int* coefficients, int n, int s, int* signs) {
    int result = 0;
    for (int i = 0; i < n; ++i) {
        result += signs[i] * coefficients[i];
    }
    return (result == s);
}

// Recursive function to generate all possible sign combinations
void generateSignCombinations(int* coefficients, int n, int s, int* signs, int index) {
    if (index == n) {
        WaitForSingleObject(hMutex, INFINITE);  // Acquire the mutex before updating the shared variable
        if (evaluateExpression(coefficients, n, s, signs)) {
            ++solutionCount;
        }
        ReleaseMutex(hMutex);  // Release the mutex after updating the shared variable
        return;
    }

    // Try both positive and negative signs for each coefficient
    signs[index] = 1;
    generateSignCombinations(coefficients, n, s, signs, index + 1);

    signs[index] = -1;
    generateSignCombinations(coefficients, n, s, signs, index + 1);
}

// Thread function
DWORD WINAPI threadFunction(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;

    int* signs = (int*)malloc(data->n * sizeof(int));
    if (signs == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        ExitThread(1);
    }

    generateSignCombinations(data->coefficients, data->n, data->s, signs, 0);

    free(signs);

    // Signal that the thread has finished
    SetEvent(hEvent);

    return 0;
}

int main() {
    LARGE_INTEGER start, end, frequency;
    QueryPerformanceFrequency(&frequency);

    FILE* inputFile = fopen("input.txt", "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Error: failed to open input file.\n");
        return 1;
    }

    int numThreads, n;
    fscanf(inputFile, "%d", &numThreads);
    fscanf(inputFile, "%d", &n);

    int* coefficients = (int*)malloc((n + 1) * sizeof(int));
    for (int i = 0; i <= n; i++) {
        fscanf(inputFile, "%d", &coefficients[i]);
    }

    fclose(inputFile);

    // Initialize synchronization objects
    hMutex = CreateMutex(NULL, FALSE, NULL);
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    HANDLE hThreads[MAX_THREADS];
    for (int i = 0; i < numThreads; i++) {
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->coefficients = (int*)malloc((n + 1) * sizeof(int));
        memcpy(data->coefficients, coefficients, (n + 1) * sizeof(int));
        data->n = n;
        data->s = coefficients[n];

        hThreads[i] = CreateThread(NULL, 0, threadFunction, data, 0, NULL);
        if (hThreads[i] == NULL) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    QueryPerformanceCounter(&start);

    // Wait for all threads to finish
    WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);

    QueryPerformanceCounter(&end);

    // Output results to files
    FILE* output = fopen("output.txt", "w");
    fprintf(output, "%d\n%d\n%d\n", numThreads, n, solutionCount);
    fclose(output);

    FILE* timeFile = fopen("time.txt", "w");
    fprintf(timeFile, "%.6f\n", (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000);
    fclose(timeFile);

    // Clean up
    CloseHandle(hMutex);
    CloseHandle(hEvent);

    for (int i = 0; i < numThreads; i++) {
        CloseHandle(hThreads[i]);
    }

    free(coefficients);

    return 0;
}
