cat > release-libpolycall.sh << 'EOF'
#!/bin/bash
VERSION=$1
TITLE="LibPolyCall ${VERSION}"

if [ -z "$VERSION" ]; then
    echo "Usage: ./release-libpolycall.sh v1.2.0"
    exit 1
fi

# Create release with changelog
gh release create $VERSION \
  --title "$TITLE" \
  --notes-file CHANGELOG.md \
  --generate-notes

# Upload assets if they exist
if [ -f "libpolycall-${VERSION}.tar.gz" ]; then
    gh release upload $VERSION libpolycall-${VERSION}.tar.gz
fi

echo "✅ Release $VERSION created!"
EOF

chmod +x release-libpolycall.sh
