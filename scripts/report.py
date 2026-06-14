#!/usr/bin/env python3

import json
import os
import sys
from pathlib import Path
from datetime import datetime
import statistics

def get_file_size(filepath):
    """Return size of file in bytes, or None if file doesn't exist."""
    if filepath and Path(filepath).exists():
        print(filepath, Path(filepath).stat().st_size)
        return Path(filepath).stat().st_size
    return None

def calculate_compression_ratio(original_size, compressed_size):
    """Calculate compression ratio (compressed/original)."""
    if original_size is None or compressed_size is None or original_size == 0:
        return float('inf')
    return compressed_size / original_size

def analyze_single_file(run_dir, results_file):
    """Analyze hyperfine results for a single test file."""
    with open(results_file, 'r') as f:
        data = json.load(f)

    test_filename = Path(results_file).stem
    original_file = Path(run_dir / test_filename)
    original_size = get_file_size(original_file)

    results = []
    for cmd_info in data['results']:
        # Extract compressor name from command line
        command = cmd_info['command']
        if '-o' in command:
            compressor = 'hfm'
            compressed_file = Path(run_dir / f"{test_filename}.hfm")
        elif 'gzip' in command:
            compressor = 'gzip'
            compressed_file = Path(run_dir / f"{test_filename}.gz")
        elif 'bzip2' in command:
            compressor = 'bzip2'
            compressed_file = Path(run_dir / f"{test_filename}.bz2")
        elif 'xz' in command:
            compressor = 'xz'
            compressed_file = Path(run_dir / f"{test_filename}.xz")
        elif 'zstd' in command:
            compressor = 'zstd'
            compressed_file = Path(run_dir / f"{test_filename}.zst")
        else:
            continue

        compressed_size = get_file_size(compressed_file)
        ratio = calculate_compression_ratio(original_size, compressed_size)

        # Calculate statistics from hyperfine output
        times = cmd_info['times']
        mean = statistics.mean(times)
        stddev = statistics.stdev(times) if len(times) > 1 else 0.0

        results.append({
            'file': test_filename,
            'compressor': compressor,
            'mean_time': mean,
            'stddev_time': stddev,
            'compressed_size': compressed_size,
            'compression_ratio': ratio,
            'raw_times': times
        })

    return results

def generate_markdown_report(all_results, output_file):
    """Generate a Markdown report from analysis results."""
    with open(output_file, 'w') as f:
        f.write(f"# Benchmark Report: HFM Compression Tool\n\n")
        f.write(f"*Generated on: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}*\n\n")

        # Summary table
        f.write("## Summary of Compression Performance\n\n")
        f.write("| File | Compressor | Mean Time (s) | Std Dev (s) | Compressed Size (bytes) | Compression Ratio |\n")
        f.write("|------|------------|---------------|-------------|------------------------|-------------------|\n")
        for res in all_results:
            f.write(f"| {res['file']} | {res['compressor']} | "
                    f"{res['mean_time']:.4f} | {res['stddev_time']:.4f} | "
                    f"{res['compressed_size']} | {res['compression_ratio']:.4f} |\n")

        # Detailed section per file
        f.write("\n## Detailed Analysis by Test File\n\n")
        for res in all_results:
            f.write(f"### {res['file']}\n\n")
            f.write(f"- **Compressor**: `{res['compressor']}`\n")
            f.write(f"- **Mean compression time**: `{res['mean_time']:.4f}` seconds\n")
            f.write(f"- **Standard deviation**: `{res['stddev_time']:.4f}` seconds\n")
            f.write(f"- **Compressed size**: `{res['compressed_size']}` bytes\n")
            f.write(f"- **Compression ratio**: `{res['compression_ratio']:.4f}`\n\n")

def main():
    if len(sys.argv) != 2:
        print("Usage: report.py <run_directory>")
        sys.exit(1)

    run_dir = Path(sys.argv[1])
    if not run_dir.is_dir():
        print(f"Error: {run_dir} is not a directory.")
        sys.exit(1)

    all_results = []
    for json_file in run_dir.glob("*.json"):
        try:
            file_results = analyze_single_file(run_dir, json_file)
            all_results.extend(file_results)
        except Exception as e:
            print(f"Error processing {json_file}: {e}")

    # Generate report
    report_path = run_dir / "benchmark_report.md"
    generate_markdown_report(all_results, report_path)
    print(f"Report generated: {report_path}")

if __name__ == "__main__":
    main()
