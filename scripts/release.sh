#!/bin/bash
# LibPolyCall Release Script

VERSION=$1
TITLE=$2

if [ -z "$VERSION" ] || [ -z "$TITLE" ]; then
    echo "Usage: ./release.sh <version> <title>"
    echo "Example: ./release.sh v1.1.stable.0.stable.0.stable \"LibPolyCall v1.1 - COBOL Binding\""
    exit 1
fi

# Create release
gh release create $VERSION \
  --title "$TITLE" \
  --notes-file RELEASE_NOTES.md \
  --generate-notes

echo "✅ Release $VERSION created!"
