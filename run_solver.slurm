#!/bin/bash
#SBATCH --job-name="wordle_solve"      # Name in the queue
#SBATCH --output="wordle_%j.out"       # Standard output file (%j = job ID)
#SBATCH --error="wordle_%j.err"        # Error output file
#SBATCH --partition=compute            # The standard queue
#SBATCH --nodes=1                      # ONE node (OpenMP limitation)
#SBATCH --ntasks=1                     # ONE main process
#SBATCH --cpus-per-task=48             # 48 threads for that process
#SBATCH --time=00:01:00                # Max runtime (HH:MM:SS)

module load GCC

# 2. specific OpenMP settings
export OMP_NUM_THREADS=48
export OMP_PLACES=cores    # Pins threads to specific cores
export OMP_PROC_BIND=close # Keeps threads close to cache

# 3. Run the executable
# srun handles the process management within the allocation
echo "Starting solver on $(hostname)"
srun ./build/WordleSolver
echo "Job Complete"
