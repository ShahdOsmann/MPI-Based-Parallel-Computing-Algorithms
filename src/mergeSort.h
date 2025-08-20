
#include <bits/stdc++.h>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <bits/stdc++.h>
#include <mpi.h>

using namespace std;

vector<int> take_input_merge() {
    vector<int> fullArray;
    int inputType, arraySize;
    cout << "Enter 0 for console input, 1 for file input: ";
    cin >> inputType;

    if (inputType == 0) {
        cout << "Enter size of array: ";
        cin >> arraySize;
        fullArray.resize(arraySize);
        cout << "Enter array elements: ";
        for (int i = 0; i < arraySize; ++i) cin >> fullArray[i];
    }
    else {
        string fileName;
        cout << "Enter input file name: ";
        cin >> fileName;

        ifstream inFile(fileName);
        if (!inFile.is_open()) {
            cerr << "Error opening file.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int arraySize;
        inFile >> arraySize;
        fullArray.resize(arraySize);
        for (int i = 0; i < arraySize; ++i) inFile >> fullArray[i];
        inFile.close();
    }
    return fullArray;
}

void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}
 
void parallelMergeSort(int rank, int size) {
    int n = 0;
    vector<int>fullArray;
    if (rank == 0) {
        fullArray = take_input_merge();
        n = fullArray.size();
    }
     
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int localSize = n / size;
    vector<int> localArray(localSize);
     
    MPI_Scatter(fullArray.data(), localSize, MPI_INT,
        localArray.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);
     
    double start_parallel = MPI_Wtime();
     
    mergeSort(localArray, 0, localSize - 1);
     
    MPI_Gather(localArray.data(), localSize, MPI_INT,
        fullArray.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);
     
    double end_parallel = MPI_Wtime();

    if (rank == 0) { 
        int step = localSize;
        while (step < n) {
            for (int i = 0; i + step < n; i += 2 * step) {
                int mid = i + step - 1;
                int right = min(i + 2 * step - 1, n - 1);
                merge(fullArray, i, mid, right);
            }
            step *= 2;
        }

        cout << "\nSorted Array: ";
        for (int x : fullArray) cout << x << " ";
        cout << endl;

        double parallel_time = end_parallel - start_parallel;
        cout << "Execution Time (Parallel): " << parallel_time << " seconds\n";
    }
}
