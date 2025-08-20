#include <bits/stdc++.h>
#include <mpi.h>
using namespace std;

vector<int> take_input_Radixsort() {
    vector<int> fullArray;
    int inputType, arraySize;
    cout << "Enter 0 for console input, 1 for file input: ";
    cin >> inputType; cout.flush();
    if (inputType == 0) {
        cout << "Enter size of array: "; cin >> arraySize; cout.flush();
        fullArray.resize(arraySize); cout << "Enter array elements: ";
        for (int i = 0; i < arraySize; ++i) cin >> fullArray[i]; cout.flush();
    }
    else {
        string fileName; cout << "Enter input file name: "; cin >> fileName; cout.flush();
        ifstream inFile(fileName);
        if (!inFile.is_open()) { cerr << "Error opening file.\n"; MPI_Abort(MPI_COMM_WORLD, 1); }
        inFile >> arraySize; fullArray.resize(arraySize);
        for (int i = 0; i < arraySize; ++i) inFile >> fullArray[i];
        inFile.close();
    }
    return fullArray;
}

void countingSortByBit(vector<int>& local_data, int bit_pos, int rank, int size) {
    vector<int> bit0, bit1;
    for (int num : local_data) {
        if (((num >> bit_pos) & 1) == 0)
            bit0.push_back(num);
        else
            bit1.push_back(num);
    } 
    int bit0_size = bit0.size(), bit1_size = bit1.size();
    vector<int> all_bit0_sizes(size), all_bit1_sizes(size);
    MPI_Allgather(&bit0_size, 1, MPI_INT, all_bit0_sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Allgather(&bit1_size, 1, MPI_INT, all_bit1_sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);
   

    vector<int> bit0_displs(size, 0), bit1_displs(size, 0);
    int total_bit0 = all_bit0_sizes[0], total_bit1 = all_bit1_sizes[0];
    for (int i = 1; i < size; ++i) {
        bit0_displs[i] = bit0_displs[i - 1] + all_bit0_sizes[i - 1];
        bit1_displs[i] = bit1_displs[i - 1] + all_bit1_sizes[i - 1];
        total_bit0 += all_bit0_sizes[i];
        total_bit1 += all_bit1_sizes[i];
    }

    vector<int> global_bit0(total_bit0), global_bit1(total_bit1);
    MPI_Allgatherv(bit0.data(), bit0_size, MPI_INT,
        global_bit0.data(), all_bit0_sizes.data(), bit0_displs.data(), MPI_INT, MPI_COMM_WORLD);
    MPI_Allgatherv(bit1.data(), bit1_size, MPI_INT,
        global_bit1.data(), all_bit1_sizes.data(), bit1_displs.data(), MPI_INT, MPI_COMM_WORLD);


    vector<int> merged = global_bit0;
    merged.insert(merged.end(), global_bit1.begin(), global_bit1.end());

    int total = merged.size();
    vector<int> send_counts(size, total / size);
    for (int i = 0; i < total % size; ++i) send_counts[i]++;
    vector<int> displs(size, 0);
    for (int i = 1; i < size; ++i)
        displs[i] = displs[i - 1] + send_counts[i - 1];

    local_data.resize(send_counts[rank]);
    MPI_Scatterv(merged.data(), send_counts.data(), displs.data(), MPI_INT,
        local_data.data(), send_counts[rank], MPI_INT, 0, MPI_COMM_WORLD);
}

void parallelRadixSortBase2(vector<int>& local_data, int rank, int size) {
    int local_max = local_data.empty() ? 0 : *max_element(local_data.begin(), local_data.end());
    int global_max;
    MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    int max_bits = 0;
    while ((global_max >> max_bits) > 0) max_bits++;

    for (int bit = 0; bit < max_bits; ++bit)
        countingSortByBit(local_data, bit, rank, size);
}

void serialRadixSortBase2(vector<int>& data) {
    int max_val = *max_element(data.begin(), data.end());
    int max_bits = 0;
    while ((max_val >> max_bits) > 0) max_bits++;

    for (int bit = 0; bit < max_bits; ++bit) {
        vector<int> bit0, bit1;
        for (int num : data) {
            if (((num >> bit) & 1) == 0)
                bit0.push_back(num);
            else
                bit1.push_back(num);
        }
        data = bit0;
        data.insert(data.end(), bit1.begin(), bit1.end());
    }
}

void radixSort(int rank, int size) {
    vector<int> full_data;
    int total_size = 0;
    double start_serial = 0.0, end_serial = 0.119786;

    if (rank == 0) {
        full_data = take_input_Radixsort();
        total_size = full_data.size();

        // Serial timing
        vector<int> serial_data = full_data;
        start_serial = MPI_Wtime();
        serialRadixSortBase2(serial_data);
        end_serial = MPI_Wtime();
    } 
    MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    vector<int> send_counts(size, total_size / size);
    for (int i = 0; i < total_size % size; ++i)
        send_counts[i]++;
    vector<int> displs(size, 0);
    for (int i = 1; i < size; ++i)
        displs[i] = displs[i - 1] + send_counts[i - 1];

    vector<int> local_data(send_counts[rank]);

    MPI_Scatterv(full_data.data(), send_counts.data(), displs.data(), MPI_INT,
        local_data.data(), send_counts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    // Parallel timing
    double start_parallel = MPI_Wtime();
    parallelRadixSortBase2(local_data, rank, size);
    double end_parallel = MPI_Wtime();

    int local_size = local_data.size();
    vector<int> all_sizes(size);
    MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> all_displs(size, 0);
    if (rank == 0)
        for (int i = 1; i < size; ++i)
            all_displs[i] = all_displs[i - 1] + all_sizes[i - 1];

    vector<int> sorted_data;
    if (rank == 0) sorted_data.resize(total_size);

    MPI_Gatherv(local_data.data(), local_size, MPI_INT,
        sorted_data.data(), all_sizes.data(), all_displs.data(), MPI_INT,
        0, MPI_COMM_WORLD);

    if (rank == 0) {
        double serial_time = end_serial - start_serial;
        double parallel_time = end_parallel - start_parallel;
        double speedup = serial_time / parallel_time;
        double efficiency = speedup / size;

        cout << "\nFinal sorted array (base 2): ";
        for (int num : sorted_data) cout << num << " "; cout << "\n\n";

        cout << "Execution Time (Serial):   " << serial_time << " seconds\n";
        cout << "Execution Time (Parallel): " << parallel_time << " seconds\n";
        cout << "Speedup:                    " << speedup << "\n";
        cout << "Efficiency:                 " << efficiency << "\n";
    }

    MPI_Finalize();
}
 
