#!/bin/bash
# LinxIpc Distribution Package Creator
# Creates a ready-to-use package with library and headers

set -e

VERSION=${1:-1.0.0}
PROJECT_ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"
PACKAGE_NAME="LinxIpc-${VERSION}"
PACKAGE_DIR="$DIST_DIR/$PACKAGE_NAME"

echo "Creating LinxIpc distribution package v${VERSION}..."

# Check if library is built
if [ ! -f "$BUILD_DIR/output/lib/libLinxIpc.a" ]; then
    echo "Error: Library not found. Please build the project first:"
    echo "  cmake -B build"
    echo "  cmake --build build"
    exit 1
fi

# Clean and create directories
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"/{lib,include,doc,examples}

# Copy library
echo "Copying library..."
cp "$BUILD_DIR/output/lib/libLinxIpc.a" "$PACKAGE_DIR/lib/"
cp "$BUILD_DIR/output/lib/libtrace.a" "$PACKAGE_DIR/lib/"

# Copy headers
echo "Copying headers..."
cp -r "$PROJECT_ROOT/LinxIpc/include"/* "$PACKAGE_DIR/include/"

# Copy trace headers
echo "Copying trace headers..."
mkdir -p "$PACKAGE_DIR/include/trace"
cp "$PROJECT_ROOT/trace/include/trace.h" "$PACKAGE_DIR/include/trace/"

# Copy documentation files
echo "Copying documentation..."
if [ -f "$PROJECT_ROOT/README.md" ]; then
    cp "$PROJECT_ROOT/README.md" "$PACKAGE_DIR/doc/"
fi
if [ -f "$PROJECT_ROOT/ARCHITECTURE.md" ]; then
    cp "$PROJECT_ROOT/ARCHITECTURE.md" "$PACKAGE_DIR/doc/"
fi
if [ -f "$PROJECT_ROOT/LICENSE.txt" ]; then
    cp "$PROJECT_ROOT/LICENSE.txt" "$PACKAGE_DIR/"
fi

# Copy distribution templates
echo "Copying distribution files..."
cp "$PROJECT_ROOT/examples/package-example/README.txt" "$PACKAGE_DIR/"
cp "$PROJECT_ROOT/examples/package-example/simple_example.cpp" "$PACKAGE_DIR/examples/"
cp "$PROJECT_ROOT/examples/package-example/CMakeLists.txt" "$PACKAGE_DIR/examples/"

# Create packages
echo "Creating archives..."
cd "$DIST_DIR"

# Create tar.gz
tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
echo "Created: $DIST_DIR/${PACKAGE_NAME}.tar.gz"

# Show package info
echo ""
echo "Package created successfully!"
echo "===================="
ls -lh "$DIST_DIR"/*.tar.gz 2>/dev/null
echo ""
echo "Package contents:"
tar -tzf "${PACKAGE_NAME}.tar.gz" | head -20
echo "... (more files)"
echo ""
echo "To extract: tar -xzf ${PACKAGE_NAME}.tar.gz"
