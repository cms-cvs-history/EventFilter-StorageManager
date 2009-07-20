#!/bin/sh

./globalConfigure.sh

echo ""
echo "========================================"
echo "Setting run numbers..."
echo "========================================"
./setRunNumbers.sh
echo ""

./globalEnable.sh
