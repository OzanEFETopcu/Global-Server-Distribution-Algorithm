import psutil
import MonteCarloSim as simulation
import time
from typing import List
import multiprocessing as mp
import ResourceMonitor as rm
import argparse



def run_monte_carlo_simulation(params: dict)->float:
    return simulation.monte_carlo_var(
        params['num_simulations'],
        params['num_days'],
        params['initial_price'],
        params['daily_volatility'],
        params['confidence_level']
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--process_num', dest='process_num', type=int, help='Define if single or multiple processes will be run and analyzed')
    args = parser.parse_args()
    simulation_params = {
        'num_simulations': 1000000,
        'num_days': 252,
        'initial_price': 100,
        'daily_volatility': 0.01,
        'confidence_level': 0.95
    }

    if isinstance(args.process_num, int):
        num_processes = args.process_num
    else:
        print("Invalid argument value, enter a integer for resource monitoring. \nUsing single monitoring type to continue")
        num_processes = 1

    # Initialize and start resource monitoring
    monitor = rm.ResourceMonitor()

    # Start monitoring before creating the process pool
    monitor.start()

    # Run parallel simulations
    start_time = time.time()

    with mp.Pool(processes=num_processes) as pool:
        # Wait a short moment for processes to start
        time.sleep(0.1)

        # Get all child process IDs
        parent = psutil.Process()
        children = parent.children(recursive=True)

        # Add all process IDs to monitor
        for child in children:
            monitor.add_process_id(child.pid)

        # Create a list of identical parameter dictionaries for each process
        params_list = [simulation_params.copy() for _ in range(num_processes)]
        results = pool.map(run_monte_carlo_simulation, params_list)

        end_time = time.time()

        # Stop monitoring and get statistics
        monitor.stop()
        avg_cpu, avg_memory = monitor.get_stats()



    # Print results
    print(f"\nParallel Simulation Results:")
    print(f"Number of processes: {num_processes}")
    print(f"VaR results from each process:")
    for i, var in enumerate(results):
        print(f"Process {i+1}: ${var:.2f}")

    print(f"\nPerformance Metrics:")
    print(f"Total execution time: {end_time - start_time:.2f} seconds")
    print(f"Average total memory usage: {avg_memory:.1f} MB")
    print(f"\nCPU Usage Pattern:")
    print(f"Average total CPU usage: {avg_cpu:.1f}%")
    print(f"Peak CPU usage: {max(monitor.cpu_measurements):.1f}%")
    print(f"Minimum CPU usage: {min(monitor.cpu_measurements):.1f}%")

    # Graph the resource usage
    monitor.graph_stats()

if __name__ == "__main__":
    # Required for Windows
    mp.freeze_support()
    main()

