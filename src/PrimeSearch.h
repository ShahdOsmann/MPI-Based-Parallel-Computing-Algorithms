#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

bool isPrime(int n)
{
    if (n == 0 || n == 1)return false;
    for (int i = 2; i <= n / i; i += 1 + (1 & i))
        if (n % i == 0)return false;
    return true;
}


void primeSearch(int rank, int size) {
    int L, R;
    double start_serial = 0.0, end_serial = 0.008045;
    vector<int> serial_primes; 
    if (rank == 0) {
        cout << "Enter the start of the range: ";
        cin >> L;
        cout << "Enter the end of the range: ";
        cin >> R;

        // --- Serial execution (only on rank 0) ---
        start_serial = MPI_Wtime();
        for (int num = L; num <= R; ++num) {
            if (isPrime(num))
                serial_primes.push_back(num);
        }
        end_serial = MPI_Wtime();
    }

    MPI_Bcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&R, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int total_numbers = R - L + 1;
    int local_size = total_numbers / size;
    int remainder = total_numbers % size;

    int local_start = L + rank * local_size + min(rank, remainder);
    int local_end = local_start + local_size - 1;
    if (rank < remainder) local_end++;

    MPI_Barrier(MPI_COMM_WORLD);  // Ensure all ranks start parallel timing together
    double start_parallel = MPI_Wtime();

    vector<int> local_primes;
    for (int num = local_start; num <= local_end; ++num)
        if (isPrime(num))
            local_primes.push_back(num);

    double end_parallel = MPI_Wtime();

    if (rank == 0) {
        int total_prime_numbers = local_primes.size();
        cout << "\nPrime numbers between " << L << " and " << R << ":\n";
        for (int p : local_primes) cout << p << " ";

        for (int i = 1; i < size; ++i) {
            int recv_count;
            MPI_Recv(&recv_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            vector<int> recv_buffer(recv_count);
            MPI_Recv(recv_buffer.data(), recv_count, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int p : recv_buffer) cout << p << " ";
            total_prime_numbers += recv_count;
        }

        cout << "\nTotal number of prime numbers is: " << total_prime_numbers << "\n\n";

        // --- Performance Metrics ---
        double serial_time = end_serial - start_serial;
        double parallel_time = end_parallel - start_parallel;
        double speedup = serial_time / parallel_time;
        double efficiency = speedup / size;

        cout << fixed << setprecision(6);
        cout << "Execution Time (Serial):   " << serial_time << " seconds\n";
        cout << "Execution Time (Parallel): " << parallel_time << " seconds\n";
        cout << "Speedup:                    " << speedup << "\n";
        cout << "Efficiency:                 " << efficiency << "\n";
    }
    else {
        int count = local_primes.size();
        MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(local_primes.data(), count, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
