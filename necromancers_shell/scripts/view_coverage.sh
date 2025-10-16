#!/bin/bash
# Quick script to view coverage reports

echo "Necromancer's Shell - Coverage Viewer"
echo "======================================"
echo ""

if [ ! -f "coverage_html/index.html" ]; then
    echo "Error: Coverage report not found!"
    echo "Run 'make coverage' first to generate the report."
    exit 1
fi

echo "Coverage reports available:"
echo "  1. HTML report: coverage_html/index.html"
echo "  2. Text summary: Run 'python3 scripts/coverage_report.py'"
echo ""

# Try to open in browser
if command -v xdg-open >/dev/null 2>&1; then
    echo "Opening coverage report in browser..."
    xdg-open coverage_html/index.html
elif command -v open >/dev/null 2>&1; then
    echo "Opening coverage report in browser..."
    open coverage_html/index.html
else
    echo "Starting web server on http://localhost:8000"
    echo "Press Ctrl+C to stop"
    python3 -m http.server 8000 -d coverage_html
fi
