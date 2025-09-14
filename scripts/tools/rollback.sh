#!/bin/bash
# LibPolyCall Rollback Script

CURRENT_TAG=$(git describe --tags --abbrev=0)
PREVIOUS_TAG=$(git describe --tags --abbrev=0 ${CURRENT_TAG}^)

echo "🔄 LibPolyCall Rollback Tool"
echo "Current: $CURRENT_TAG"
echo "Rollback to: $PREVIOUS_TAG"
echo ""
read -p "Proceed? (y/n): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    git checkout $PREVIOUS_TAG
    make clean && make all
    echo "✅ Rolled back to $PREVIOUS_TAG"
else
    echo "❌ Rollback cancelled"
fi
