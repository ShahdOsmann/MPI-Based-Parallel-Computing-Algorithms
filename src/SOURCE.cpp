#include <mpi.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Search.h"
#include "PrimeSearch.h"
#include "RadixSort.h" 
#include "mergeSort.h" 
#include <iostream>

using namespace std;
int main(int argc, char* argv[]) {
    int rank = -1, size, choice = 0, status;
    MPI_Status STATUS;
    MPI_Init(&argc, &argv);  // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Get the total number of processes

    if (!rank)
    {
        printf("===============================================\n");
        printf("Welcome to Parallel Algorithm Simulation with MPI\n");
        printf("===============================================\n");
        printf("Please choose an algorithm to execute:\n");
        printf("01 - linear Search\n");
        printf("02 - Prime Number Finding\n"); 
        printf("03 - Radix Sort\n");
        printf("04 - Merge Sort\n"); 
        printf("\nEnter the number of the algorithm to run: ");
        cout.flush();
        cin >> choice;
        for (int i = 1; i < size; i++) {
            MPI_Send(&choice, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
        }
    }
    else
        MPI_Recv(&choice, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &STATUS);

    switch (choice)
    {
    case 1:
        Search(rank, size);
        break;
    case 2:
        primeSearch(rank, size);
        break; 
    case 3:
        radixSort(rank, size);
        break;
    case 4:
        parallelMergeSort(rank, size);
        break; 
    default:
        cout << "wrong choice"; cout.flush();
    }

}
// 11 170 45 75 90 802 24 2 66 0 20 12

