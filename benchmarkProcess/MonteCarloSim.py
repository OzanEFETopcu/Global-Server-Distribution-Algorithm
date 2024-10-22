import numpy as np
import time
import psutil
import os
from threading import Thread
import statistics
import pandas as pd
import matplotlib.pyplot as plt


class ResourceMonitor:
    def __init__(self):
        self.is_running = False
        self.cpu_measurements = []
        self.memory_measurements = []
        self.timestamps = []

    def measure_resources(self):
        process = psutil.Process(os.getpid())
        start_time = time.time()
        while self.is_running:
            current_time = time.time() - start_time
            self.timestamps.append(current_time)
            self.cpu_measurements.append(process.cpu_percent())
            self.memory_measurements.append(process.memory_info().rss / 1024 / 1024)  # MB
            time.sleep(0.1)  # Sample every 100ms

    def start(self):
        self.is_running = True
        self.monitor_thread = Thread(target=self.measure_resources)
        self.monitor_thread.start()

    def stop(self):
        self.is_running = False
        self.monitor_thread.join()

    def get_stats(self):
        if not self.cpu_measurements:
            return 0, 0

        # Remove the first few measurements as they might be inaccurate
        cpu_measurements = self.cpu_measurements[5:]
        memory_measurements = self.memory_measurements[5:]

        if not cpu_measurements:
            return 0, 0

        avg_cpu = statistics.mean(cpu_measurements)
        avg_memory = statistics.mean(memory_measurements)
        return avg_cpu, avg_memory

    def graph_stats(self):
        if not self.cpu_measurements or len(self.cpu_measurements) < 5:
            print("Not enough measurements to create a graph")
            return

        # Create figure with two subplots
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

        # Remove first few measurements
        cpu_data = self.cpu_measurements[5:]
        memory_data = self.memory_measurements[5:]
        time_data = self.timestamps[5:]

        # Create DataFrames
        df = pd.DataFrame({
            'Time (seconds)': time_data,
            'CPU Usage (%)': cpu_data,
            'Memory Usage (MB)': memory_data
        })

        # Plot CPU usage
        ax1.plot(df['Time (seconds)'], df['CPU Usage (%)'], 'b-', alpha=0.7)
        ax1.fill_between(df['Time (seconds)'], df['CPU Usage (%)'], alpha=0.3)
        ax1.set_title('CPU Usage Over Time')
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('CPU Usage (%)')
        ax1.grid(True)

        # Plot Memory usage
        ax2.plot(df['Time (seconds)'], df['Memory Usage (MB)'], 'r-', alpha=0.7)
        ax2.fill_between(df['Time (seconds)'], df['Memory Usage (MB)'], alpha=0.3)
        ax2.set_title('Memory Usage Over Time')
        ax2.set_xlabel('Time (seconds)')
        ax2.set_ylabel('Memory Usage (MB)')
        ax2.grid(True)

        plt.tight_layout()
        plt.show()

def monte_carlo_var(num_simulations, num_days, initial_price, daily_volatility, confidence_level):
    # Generate random daily returns
    daily_returns = np.random.normal(0, daily_volatility, (num_simulations, num_days))

    # Calculate price paths
    price_paths = initial_price * np.exp(np.cumsum(daily_returns, axis=1))

    # Calculate the final price for each simulation
    final_prices = price_paths[:, -1]

    # Sort the final prices
    sorted_final_prices = np.sort(final_prices)

    # Calculate VaR
    var_index = int(num_simulations * (1 - confidence_level))
    var = initial_price - sorted_final_prices[var_index]

    return var

def main():
    # Simulation parameters
    num_simulations = 1000000  # Increased for better demonstration
    num_days = 252*2  # Trading days in a year
    initial_price = 100
    daily_volatility = 0.01
    confidence_level = 0.95

    # Initialize and start resource monitoring
    monitor = ResourceMonitor()
    monitor.start()

    # Run simulation
    start_time = time.time()
    var = monte_carlo_var(num_simulations, num_days, initial_price, daily_volatility, confidence_level)
    end_time = time.time()

    # Stop monitoring and get statistics
    monitor.stop()
    avg_cpu, avg_memory = monitor.get_stats()


    # Print results
    print(f"\nSimulation Results:")
    print(f"Number of simulations: {num_simulations:,}")
    print(f"Number of days: {num_days:,}")
    print(f"Value at Risk (VaR) at {confidence_level*100}% confidence: ${var:.2f}")
    print(f"\nPerformance Metrics:")
    print(f"Execution time: {end_time - start_time:.2f} seconds")
    print(f"Average CPU usage: {avg_cpu:.1f}%")
    print(f"Average Memory usage: {avg_memory:.1f} MB")

    # Print CPU usage pattern
    if monitor.cpu_measurements:
        print(f"\nCPU Usage Pattern:")
        print(f"Peak CPU usage: {max(monitor.cpu_measurements):.1f}%")
        print(f"Minimum CPU usage: {min(monitor.cpu_measurements):.1f}%")
        print(f"Number of measurements: {len(monitor.cpu_measurements)}")

    monitor.graph_stats()

if __name__ == "__main__":
    main()