import os
import time
import numpy as np
import psutil
from threading import Thread
import statistics
import pandas as pd
import matplotlib.pyplot as plt
from typing import Set, Tuple


class ResourceMonitor:
    def __init__(self):
        self.is_running = False
        self.cpu_measurements = []
        self.memory_measurements = []
        self.timestamps = []
        self.process_ids: Set[int] = set()

    def measure_resources(self):
        self.start_time = time.time()
        while self.is_running:
            current_time = time.time() - self.start_time
            # Initialize measurements for this iteration
            total_cpu = 0
            total_memory = 0
            # Measure for the main process and all children
            for pid in list(self.process_ids):
                try:
                    process = psutil.Process(pid)
                    process.cpu_percent()
                    time.sleep(0.1)
                    total_cpu += process.cpu_percent()
                    total_memory += process.memory_info().rss / 1024 / 1024  # MB
                except (psutil.NoSuchProcess, psutil.AccessDenied) as e:
                    print(f"Warning: Could not access process {pid}: {str(e)}")
                    self.process_ids.discard(pid)  # Remove invalid process ID

            self.timestamps.append(current_time)
            self.cpu_measurements.append(total_cpu)
            self.memory_measurements.append(total_memory)

            time.sleep(0.1)  # Sample every 100ms

    def start(self):
        self.is_running = True
        self.monitor_thread = Thread(target=self.measure_resources)
        self.monitor_thread.start()

    def stop(self):
        self.is_running = False
        self.monitor_thread.join()

    def add_process_id(self, pid: int):
        print(f"Added process {pid} to monitoring")
        self.process_ids.add(pid)

    def get_stats(self):
        if not self.cpu_measurements:
            return 0, 0

        # Remove the first few measurements as they might be inaccurate
        cpu_measurements = self.cpu_measurements[5:] if len(self.cpu_measurements) > 5 else self.cpu_measurements
        memory_measurements = self.memory_measurements[5:] if len(self.memory_measurements) > 5 else self.memory_measurements

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

        # Remove first few measurements if possible
        cpu_data = self.cpu_measurements[5:] if len(self.cpu_measurements) > 5 else self.cpu_measurements
        memory_data = self.memory_measurements[5:] if len(self.memory_measurements) > 5 else self.memory_measurements
        time_data = self.timestamps[5:] if len(self.timestamps) > 5 else self.timestamps

        # Create DataFrames
        df = pd.DataFrame({
            'Time (seconds)': time_data,
            'CPU Usage (%)': cpu_data, #np.cumsum(cpu_data),
            'Memory Usage (MB)': memory_data #np.cumsum(memory_data)
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


