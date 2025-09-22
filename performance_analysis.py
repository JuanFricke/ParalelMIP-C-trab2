#!/usr/bin/env python3
"""
Performance Analysis Script for Parallel Spotify Data Processing
This script analyzes the performance metrics and generates reports.
"""

import subprocess
import time
import json
import matplotlib.pyplot as plt
import numpy as np
from typing import Dict, List, Tuple
import os

class PerformanceAnalyzer:
    def __init__(self):
        self.results = {}
        
    def run_benchmark(self, num_processes: int, num_runs: int = 3) -> Dict:
        """
        Run benchmark with specified number of processes.
        
        Args:
            num_processes: Number of MPI processes to use
            num_runs: Number of runs to average
            
        Returns:
            Dictionary with performance metrics
        """
        print(f"Running benchmark with {num_processes} processes ({num_runs} runs)...")
        
        times = []
        for run in range(num_runs):
            print(f"  Run {run + 1}/{num_runs}")
            
            start_time = time.time()
            try:
                result = subprocess.run(
                    ['mpirun', '-np', str(num_processes), './spotify_processor'],
                    capture_output=True,
                    text=True,
                    timeout=300  # 5 minute timeout
                )
                
                end_time = time.time()
                execution_time = end_time - start_time
                
                if result.returncode == 0:
                    times.append(execution_time)
                    print(f"    Execution time: {execution_time:.2f}s")
                else:
                    print(f"    Error: {result.stderr}")
                    
            except subprocess.TimeoutExpired:
                print(f"    Timeout after 5 minutes")
            except Exception as e:
                print(f"    Exception: {e}")
        
        if times:
            avg_time = sum(times) / len(times)
            min_time = min(times)
            max_time = max(times)
            
            return {
                'num_processes': num_processes,
                'avg_time': avg_time,
                'min_time': min_time,
                'max_time': max_time,
                'times': times,
                'successful_runs': len(times)
            }
        else:
            return {
                'num_processes': num_processes,
                'avg_time': None,
                'min_time': None,
                'max_time': None,
                'times': [],
                'successful_runs': 0
            }
    
    def run_scaling_benchmark(self, process_counts: List[int] = None) -> Dict:
        """
        Run scaling benchmark with different numbers of processes.
        
        Args:
            process_counts: List of process counts to test
            
        Returns:
            Dictionary with scaling results
        """
        if process_counts is None:
            process_counts = [1, 2, 4, 8, 16]
        
        print("=== Running Scaling Benchmark ===")
        
        # Ensure the executable is built
        if not os.path.exists('./spotify_processor'):
            print("Building executable...")
            subprocess.run(['make', 'clean'], check=True)
            subprocess.run(['make'], check=True)
        
        results = {}
        for num_procs in process_counts:
            print(f"\nTesting with {num_procs} processes...")
            result = self.run_benchmark(num_procs)
            results[num_procs] = result
        
        self.results = results
        return results
    
    def calculate_speedup(self, results: Dict) -> Dict:
        """Calculate speedup and efficiency metrics."""
        if not results:
            return {}
        
        # Get baseline (sequential) time
        baseline_time = None
        for num_procs in sorted(results.keys()):
            if results[num_procs]['avg_time'] is not None:
                baseline_time = results[num_procs]['avg_time']
                break
        
        if baseline_time is None:
            return {}
        
        speedup_results = {}
        for num_procs, result in results.items():
            if result['avg_time'] is not None:
                speedup = baseline_time / result['avg_time']
                efficiency = speedup / num_procs
                
                speedup_results[num_procs] = {
                    'speedup': speedup,
                    'efficiency': efficiency,
                    'time': result['avg_time']
                }
        
        return speedup_results
    
    def generate_report(self, results: Dict, output_file: str = "performance_report.txt"):
        """Generate a detailed performance report."""
        speedup_results = self.calculate_speedup(results)
        
        with open(output_file, 'w') as f:
            f.write("=== PARALLEL SPOTIFY DATA PROCESSING - PERFORMANCE REPORT ===\n\n")
            
            f.write("EXECUTION TIMES:\n")
            f.write("-" * 50 + "\n")
            for num_procs in sorted(results.keys()):
                result = results[num_procs]
                if result['avg_time'] is not None:
                    f.write(f"Processes: {num_procs:2d} | "
                           f"Avg: {result['avg_time']:6.2f}s | "
                           f"Min: {result['min_time']:6.2f}s | "
                           f"Max: {result['max_time']:6.2f}s | "
                           f"Runs: {result['successful_runs']}\n")
                else:
                    f.write(f"Processes: {num_procs:2d} | FAILED\n")
            
            f.write("\nSPEEDUP AND EFFICIENCY:\n")
            f.write("-" * 50 + "\n")
            for num_procs in sorted(speedup_results.keys()):
                result = speedup_results[num_procs]
                f.write(f"Processes: {num_procs:2d} | "
                       f"Speedup: {result['speedup']:5.2f}x | "
                       f"Efficiency: {result['efficiency']:5.2f} | "
                       f"Time: {result['time']:6.2f}s\n")
            
            f.write("\nANALYSIS:\n")
            f.write("-" * 50 + "\n")
            
            # Find best performance
            best_speedup = max(speedup_results.values(), key=lambda x: x['speedup'])
            best_procs = [k for k, v in speedup_results.items() if v['speedup'] == best_speedup['speedup']][0]
            
            f.write(f"Best speedup: {best_speedup['speedup']:.2f}x with {best_procs} processes\n")
            f.write(f"Best efficiency: {max(speedup_results.values(), key=lambda x: x['efficiency'])['efficiency']:.2f}\n")
            
            # Performance analysis
            f.write("\nPERFORMANCE INSIGHTS:\n")
            f.write("-" * 50 + "\n")
            
            if len(speedup_results) > 1:
                # Check for diminishing returns
                procs = sorted(speedup_results.keys())
                speedups = [speedup_results[p]['speedup'] for p in procs]
                
                if len(speedups) >= 2:
                    improvement_2_to_4 = speedups[1] - speedups[0] if len(speedups) > 1 else 0
                    improvement_4_to_8 = speedups[2] - speedups[1] if len(speedups) > 2 else 0
                    
                    f.write(f"Speedup improvement from 1 to 2 processes: {improvement_2_to_4:.2f}x\n")
                    f.write(f"Speedup improvement from 2 to 4 processes: {improvement_4_to_8:.2f}x\n")
                    
                    if improvement_4_to_8 < improvement_2_to_4 * 0.5:
                        f.write("→ Diminishing returns observed with higher process counts\n")
                        f.write("→ Communication overhead may be limiting scalability\n")
                    else:
                        f.write("→ Good scalability observed\n")
            
            f.write("\nRECOMMENDATIONS:\n")
            f.write("-" * 50 + "\n")
            f.write("1. For optimal performance, use the process count with best speedup\n")
            f.write("2. Consider communication overhead when scaling to many processes\n")
            f.write("3. Monitor system resources (CPU, memory, network) during execution\n")
            f.write("4. Profile the application to identify bottlenecks\n")
        
        print(f"Performance report saved to {output_file}")
    
    def plot_results(self, results: Dict, output_file: str = "performance_plot.png"):
        """Generate performance plots."""
        if not results:
            print("No results to plot")
            return
        
        speedup_results = self.calculate_speedup(results)
        
        if not speedup_results:
            print("No valid results for plotting")
            return
        
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
        
        # Plot 1: Execution Time
        procs = sorted(results.keys())
        times = [results[p]['avg_time'] for p in procs if results[p]['avg_time'] is not None]
        valid_procs = [p for p in procs if results[p]['avg_time'] is not None]
        
        ax1.plot(valid_procs, times, 'bo-', linewidth=2, markersize=8)
        ax1.set_xlabel('Number of Processes')
        ax1.set_ylabel('Execution Time (seconds)')
        ax1.set_title('Execution Time vs Number of Processes')
        ax1.grid(True, alpha=0.3)
        
        # Plot 2: Speedup and Efficiency
        speedup_procs = sorted(speedup_results.keys())
        speedups = [speedup_results[p]['speedup'] for p in speedup_procs]
        efficiencies = [speedup_results[p]['efficiency'] for p in speedup_procs]
        
        ax2.plot(speedup_procs, speedups, 'ro-', linewidth=2, markersize=8, label='Speedup')
        ax2.plot(speedup_procs, efficiencies, 'go-', linewidth=2, markersize=8, label='Efficiency')
        ax2.axline((0, 0), slope=1, color='k', linestyle='--', alpha=0.5, label='Ideal Speedup')
        ax2.set_xlabel('Number of Processes')
        ax2.set_ylabel('Speedup / Efficiency')
        ax2.set_title('Speedup and Efficiency vs Number of Processes')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Performance plot saved to {output_file}")

def main():
    """Main function for command-line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Performance Analysis for Parallel Spotify Processing')
    parser.add_argument('--processes', nargs='+', type=int, default=[1, 2, 4, 8],
                       help='Number of processes to test (default: 1 2 4 8)')
    parser.add_argument('--runs', type=int, default=3,
                       help='Number of runs per configuration (default: 3)')
    parser.add_argument('--output', default='performance_report.txt',
                       help='Output file for report (default: performance_report.txt)')
    parser.add_argument('--plot', default='performance_plot.png',
                       help='Output file for plot (default: performance_plot.png)')
    
    args = parser.parse_args()
    
    analyzer = PerformanceAnalyzer()
    
    print("Starting performance analysis...")
    print(f"Testing with processes: {args.processes}")
    print(f"Runs per configuration: {args.runs}")
    
    # Run benchmarks
    results = analyzer.run_scaling_benchmark(args.processes)
    
    # Generate report
    analyzer.generate_report(results, args.output)
    
    # Generate plots (if matplotlib is available)
    try:
        analyzer.plot_results(results, args.plot)
    except ImportError:
        print("Matplotlib not available, skipping plot generation")
        print("Install with: pip install matplotlib")

if __name__ == "__main__":
    main()
