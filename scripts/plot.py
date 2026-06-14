#!/usr/bin/env python3

import json
import sys
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm

def plot_comparison(results_by_file, run_dir):
    """Create a bar chart comparing mean compression times."""
    files = list(results_by_file.keys())
    compressors = sorted({comp for v in results_by_file.values() for comp, _ in v.items()})

    # Prepare data matrix
    data = {comp: [] for comp in compressors}
    for fname in files:
        for comp in compressors:
            if comp in results_by_file[fname]:
                data[comp].append(results_by_file[fname][comp]['mean_time'])
            else:
                data[comp].append(0)

    x = np.arange(len(files))
    width = 0.8 / len(compressors)
    multiplier = 0

    fig, ax = plt.subplots(figsize=(12, 6))
    for comp in compressors:
        offset = width * multiplier
        rects = ax.bar(x + offset, data[comp], width, label=comp)
        ax.bar_label(rects, fmt='%.2f', padding=3)
        multiplier += 1

    ax.set_ylabel('Mean Compression Time (seconds)')
    ax.set_title('Compression Speed Comparison')
    ax.set_xticks(x + width * (len(compressors) - 1) / 2)
    ax.set_xticklabels(files)
    ax.legend(loc='upper left', ncols=len(compressors))
    ax.grid(True, axis='y', linestyle='--', alpha=0.7)

    plt.tight_layout()
    plot_path = run_dir / "speed_comparison.png"
    plt.savefig(plot_path, dpi=150)
    print(f"Speed comparison plot saved to {plot_path}")
    plt.close()

def plot_normal_distribution(results_by_file, run_dir):
    """Plot histogram of raw times for HFM with fitted normal distribution."""
    hfm_data = {}
    for fname, compressors in results_by_file.items():
        if 'hfm' in compressors:
            hfm_data[fname] = compressors['hfm']['raw_times']

    if not hfm_data:
        print("No HFM data found for distribution plot.")
        return

    n_files = len(hfm_data)
    fig, axes = plt.subplots(1, n_files, figsize=(5*n_files, 4))
    if n_files == 1:
        axes = [axes]

    for ax, (fname, times) in zip(axes, hfm_data.items()):
        ax.hist(times, bins='auto', density=True, alpha=0.6, color='skyblue', edgecolor='black', label='Measured')
        mu, std = np.mean(times), np.std(times)
        xmin, xmax = ax.get_xlim()
        x = np.linspace(mu - 3*std, mu + 3*std, 100)
        ax.plot(x, norm.pdf(x, mu, std), 'r-', linewidth=2, label=f'Normal: μ={mu:.3f}, σ={std:.3f}')
        ax.set_title(f'HFM: {fname}')
        ax.set_xlabel('Time (seconds)')
        ax.set_ylabel('Density')
        ax.legend()
        ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plot_path = run_dir / "hfm_normal_distribution.png"
    plt.savefig(plot_path, dpi=150)
    print(f"Normal distribution plot for HFM saved to {plot_path}")
    plt.close()

def load_results(run_dir):
    """Load all JSON results from a run directory."""
    results_by_file = {}
    for json_file in run_dir.glob("*.json"):
        fname = json_file.stem
        with open(json_file, 'r') as f:
            data = json.load(f)
        results_by_file[fname] = {}
        for cmd in data['results']:
            if '-o' in cmd['command']:
                comp = 'hfm'
            elif 'gzip' in cmd['command']:
                comp = 'gzip'
            elif 'bzip2' in cmd['command']:
                comp = 'bzip2'
            elif 'xz' in cmd['command']:
                comp = 'xz'
            elif 'zstd' in cmd['command']:
                comp = 'zstd'
            else:
                continue
            results_by_file[fname][comp] = {
                'mean_time': cmd['mean'],
                'stddev_time': cmd['stddev'],
                'raw_times': cmd['times']
            }
    return results_by_file

def main():
    if len(sys.argv) != 2:
        print("Usage: plot.py <run_directory>")
        sys.exit(1)

    run_dir = Path(sys.argv[1])
    if not run_dir.is_dir():
        print(f"Error: {run_dir} is not a directory.")
        sys.exit(1)

    results = load_results(run_dir)
    if not results:
        print("No valid result files found.")
        sys.exit(1)

    plot_comparison(results, run_dir)
    plot_normal_distribution(results, run_dir)

if __name__ == "__main__":
    main()
