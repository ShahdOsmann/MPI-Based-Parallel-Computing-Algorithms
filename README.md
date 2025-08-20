# Parallel Search and Sorting Algorithms with MPI  

## 1. Introduction  
This project implements and analyzes **parallel search and sorting algorithms** using the **Message Passing Interface (MPI)** in C++.  

The goal is to leverage parallel computing to:  
- Enhance computational efficiency.  
- Compare **serial vs parallel** execution.  
- Study **speedup** and **efficiency** of MPI-based distributed algorithms.  

The project includes the following algorithms:  
- Linear Search  
- Prime Number Finding  
- Radix Sort (Base 2)  
- Merge Sort  

Each algorithm is implemented with both **serial and parallel versions** for fair comparison.  

---

## 2. Problem Distribution and Input Handling  

### 2.1 Master-Worker Paradigm  
- **Master Process (Rank 0):**  
  - Reads input (array for sorting/searching, or range for primes) from console or file.  
  - Divides the input into chunks and distributes them across worker processes.  
  - Collects results, merges partial outputs, and prints final results and performance metrics.  

- **Worker Processes:**  
  - Receive input data from the master.  
  - Execute their portion of the algorithm (e.g., searching or sorting).  
  - Send intermediate results back to the master (using `MPI_Gather`, `MPI_Reduce`, or point-to-point communication).  

- **Dynamic Input Sizes:**  
  - Partition sizes are computed dynamically using `MPI_Comm_size` (number of processes) and the input size.  
  - Supports inputs that are not divisible by the number of processes, ensuring **load balancing**.  

---

### 2.2 Input Handling  
- **Input Format:**  
  - Arrays are read from console input or from a text file.  
  - For file input, the first line contains the array size, followed by the array elements.  
- **Distribution:**  
  - `MPI_Scatterv` is used for arrays (to handle uneven splits).  
  - `MPI_Bcast` is used to broadcast scalar values such as target elements or prime range boundaries.  
- **Scalability:**  
  - Design avoids bottlenecks in input distribution.  
  - Works with varying numbers of processes, making it suitable for both small-scale and larger-scale distributed systems.  

---

## 3. Parallel Algorithms Implementation  

### 3.1 Linear Search  
- Serial search runs on rank 0 for performance comparison.  
- The array is distributed across processes with **`MPI_Scatterv`**.  
- Each process searches its local chunk for the target element.  
- Global result is obtained using **`MPI_Allreduce`**.  
- Reports serial time, parallel time, speedup, and efficiency.  

---

### 3.2 Prime Number Search  
- User inputs a range `[L, R]`.  
- Rank 0 performs the **serial prime search** for comparison.  
- The range is divided into disjoint sub-ranges, assigned to each process.  
- Each process finds primes in its sub-range independently.  
- Results are collected at rank 0 via **point-to-point communication (MPI_Send / MPI_Recv)**.  
- Outputs the prime numbers, their count, and performance metrics.  

---

### 3.3 Radix Sort (Base 2)  
- Rank 0 performs a **serial radix sort** (base 2) for benchmarking.  
- The array is split and distributed with **`MPI_Scatterv`**.  
- Sorting proceeds bit by bit. At each step:  
  - Processes classify elements into `bit0` and `bit1`.  
  - Results are combined using **`MPI_Allgatherv`**.  
- Final results are gathered at rank 0 with **`MPI_Gatherv`**.  
- Reports sorted array, serial vs parallel execution time, speedup, and efficiency.  

---

### 3.4 Merge Sort  
- Input array is read by rank 0.  
- The array is broadcast and partitioned across processes with **`MPI_Scatter`**.  
- Each process applies a **recursive merge sort** locally.  
- Local results are gathered with **`MPI_Gather`**.  
- Rank 0 merges partially sorted segments step-by-step until the full array is sorted.  
- Prints sorted array and execution time.  

---

## 4. Performance Evaluation  
Each algorithm reports:  
- **Execution Time (Serial and Parallel)**  
- **Speedup:** ratio of serial time to parallel time.  
- **Efficiency:** speedup divided by the number of processes.  

This helps analyze how well MPI parallelization scales across different problem sizes and process counts.  

---
## 5. How to run 
```bash
- mpiexec -n <num_processes> ./mpi_project
