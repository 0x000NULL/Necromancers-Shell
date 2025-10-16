#!/usr/bin/env python3
"""
Generate a comprehensive code coverage report from gcov data
"""
import os
import re
import subprocess
from collections import defaultdict

def parse_gcov_output(gcda_file):
    """Parse gcov output for a single .gcda file"""
    try:
        result = subprocess.run(
            ['gcov', '-r', gcda_file],
            capture_output=True,
            text=True,
            cwd=os.getcwd()
        )

        output = result.stdout + result.stderr

        # Extract file name and coverage
        file_match = re.search(r"File '([^']+)'", output)
        coverage_match = re.search(r"Lines executed:(\d+\.\d+)% of (\d+)", output)

        if file_match and coverage_match:
            filename = file_match.group(1)
            percentage = float(coverage_match.group(1))
            total_lines = int(coverage_match.group(2))
            covered_lines = int(total_lines * percentage / 100)

            return {
                'file': filename,
                'percentage': percentage,
                'covered': covered_lines,
                'total': total_lines
            }
    except Exception as e:
        pass

    return None

def main():
    print("=" * 70)
    print("Test Coverage Report - Necromancer's Shell")
    print("=" * 70)
    print()

    # Find all .gcda files
    gcda_files = []
    for root, dirs, files in os.walk('build'):
        for file in files:
            if file.endswith('.gcda'):
                gcda_files.append(os.path.join(root, file))

    if not gcda_files:
        print("No coverage data found. Run 'make coverage' first.")
        return

    print(f"Found {len(gcda_files)} files with coverage data")
    print()

    # Parse all coverage data
    coverage_data = []
    for gcda_file in gcda_files:
        data = parse_gcov_output(gcda_file)
        if data:
            coverage_data.append(data)

    if not coverage_data:
        print("Could not parse coverage data")
        return

    # Calculate overall statistics
    total_lines = sum(d['total'] for d in coverage_data)
    covered_lines = sum(d['covered'] for d in coverage_data)
    overall_percentage = (covered_lines / total_lines * 100) if total_lines > 0 else 0

    print(f"Overall Coverage: {overall_percentage:.2f}%")
    print(f"  Covered Lines: {covered_lines:,} / {total_lines:,}")
    print()

    # Group by category
    categories = defaultdict(lambda: {'covered': 0, 'total': 0})
    for data in coverage_data:
        file_path = data['file']

        if '/core/' in file_path:
            category = 'Core Systems'
        elif '/commands/' in file_path:
            category = 'Command System'
        elif '/game/souls/' in file_path or '/game/minions/' in file_path:
            category = 'Game Entities'
        elif '/game/combat/' in file_path:
            category = 'Combat System'
        elif '/game/narrative/' in file_path:
            category = 'Narrative Systems'
        elif '/game/world/' in file_path:
            category = 'World Systems'
        elif '/game/progression/' in file_path:
            category = 'Progression'
        elif '/terminal/' in file_path:
            category = 'Terminal/UI'
        elif '/utils/' in file_path:
            category = 'Utilities'
        elif '/data/' in file_path:
            category = 'Data Loaders'
        else:
            category = 'Other'

        categories[category]['covered'] += data['covered']
        categories[category]['total'] += data['total']

    # Print category breakdown
    print("Coverage by Category:")
    print("-" * 70)
    print(f"{'Category':<30} {'Coverage':<15} {'Lines':<20}")
    print("-" * 70)

    for category in sorted(categories.keys()):
        stats = categories[category]
        percentage = (stats['covered'] / stats['total'] * 100) if stats['total'] > 0 else 0
        print(f"{category:<30} {percentage:>6.2f}%{'':<8} {stats['covered']:>6}/{stats['total']:<6}")

    print()

    # Top 10 best covered files
    print("Top 10 Best Covered Files:")
    print("-" * 70)
    top_files = sorted(coverage_data, key=lambda x: x['percentage'], reverse=True)[:10]
    for data in top_files:
        filename = os.path.basename(data['file'])
        print(f"  {filename:<40} {data['percentage']:>6.2f}%  ({data['covered']}/{data['total']})")

    print()

    # Files needing improvement (<50%)
    low_coverage = [d for d in coverage_data if d['percentage'] < 50]
    if low_coverage:
        low_coverage.sort(key=lambda x: x['percentage'])
        print(f"Files Needing Improvement (<50% coverage): {len(low_coverage)} files")
        print("-" * 70)
        for data in low_coverage[:15]:  # Show top 15
            filename = os.path.basename(data['file'])
            print(f"  {filename:<40} {data['percentage']:>6.2f}%  ({data['covered']}/{data['total']})")
    else:
        print("All files have >50% coverage!")

    print()
    print("=" * 70)
    print("For HTML reports, install lcov: sudo pacman -S lcov")
    print("Then run: make coverage")
    print("=" * 70)

if __name__ == '__main__':
    main()
