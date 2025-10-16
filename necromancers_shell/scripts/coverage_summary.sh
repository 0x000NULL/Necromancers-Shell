#!/bin/bash
# Generate a coverage summary for all tested code

echo "======================================"
echo "Test Coverage Summary"
echo "======================================"
echo ""

total_lines=0
covered_lines=0

# Process all .gcda files in build directory
for gcda in $(find build -name "*.gcda"); do
    # Run gcov and capture output
    output=$(gcov -r "$gcda" 2>&1 | grep "Lines executed:")

    if [ -n "$output" ]; then
        # Extract percentage and line counts
        percentage=$(echo "$output" | sed -n 's/Lines executed:\([0-9.]*\)% of \([0-9]*\)/\1/p')
        lines=$(echo "$output" | sed -n 's/Lines executed:\([0-9.]*\)% of \([0-9]*\)/\2/p')

        if [ -n "$percentage" ] && [ -n "$lines" ]; then
            covered=$(echo "$percentage * $lines / 100" | bc)
            total_lines=$((total_lines + lines))
            covered_lines=$(echo "$covered_lines + $covered" | bc)
        fi
    fi
done

# Calculate overall coverage
if [ $total_lines -gt 0 ]; then
    overall=$(echo "scale=2; $covered_lines * 100 / $total_lines" | bc)
    echo "Overall Coverage: ${overall}% ($covered_lines / $total_lines lines)"
else
    echo "No coverage data found. Run 'make coverage' first."
fi

echo ""
echo "Top covered files:"
echo "======================================"
for gcda in $(find build -name "*.gcda"); do
    gcov -r "$gcda" 2>&1 | grep -E "^(File|Lines executed)" | paste - -
done | sort -t: -k2 -nr | head -20

echo ""
echo "Files needing coverage improvement (<50%):"
echo "======================================"
for gcda in $(find build -name "*.gcda"); do
    gcov -r "$gcda" 2>&1 | grep -E "^(File|Lines executed)" | paste - -
done | awk -F'Lines executed:' '{if ($2) print $0}' | awk '{if ($3 < 50) print $0}' | sort -t: -k2 -n | head -20
