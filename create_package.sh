#!/bin/bash
# LinxIpc Distribution Package Creator
# Creates a ready-to-use package with library and headers

set -e

PROJECT_ROOT=$(cd "$(dirname "$0")" && pwd)

# Default values
VERSION=""
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/build/dist"

# Usage function
usage() {
    echo "Usage: $0 [-v version] [-b build_dir] [-d dist_dir] [-h]"
    echo ""
    echo "Options:"
    echo "  -v VERSION    Specify package version (default: auto-detect from git)"
    echo "  -b BUILD_DIR  Specify build directory (default: ./build)"
    echo "  -d DIST_DIR   Specify distribution directory (default: ./dist)"
    echo "  -h            Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                      # Use defaults"
    echo "  $0 -v 1.0.0             # Specify version"
    echo "  $0 -b build_ut          # Use different build directory"
    echo "  $0 -d /tmp/dist         # Use different dist directory"
    echo "  $0 -v 2.0.0 -b build_ut # Specify multiple options"
    exit 1
}

# Parse arguments using getopts
while getopts "v:b:d:h" opt; do
    case $opt in
        v)
            VERSION="$OPTARG"
            ;;
        b)
            BUILD_DIR="$OPTARG"
            ;;
        d)
            DIST_DIR="$OPTARG"
            ;;
        h)
            usage
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            usage
            ;;
    esac
done

# Determine version from git tags if not provided
if [ -z "$VERSION" ]; then
    # Try to get exact tag on current commit
    if GIT_TAG=$(git describe --exact-match --tags 2>/dev/null); then
        VERSION="$GIT_TAG"
    else
        # Get latest tag + commit SHA
        if LATEST_TAG=$(git describe --tags --abbrev=0 2>/dev/null); then
            # Get short commit SHA
            COMMIT_SHA=$(git rev-parse --short HEAD 2>/dev/null)
            VERSION="${LATEST_TAG}-${COMMIT_SHA}"
        else
            VERSION="1.0.0"
        fi
    fi
fi

PACKAGE_NAME="LinxIpc-${VERSION}"
PACKAGE_DIR="$DIST_DIR/$PACKAGE_NAME"

echo "Creating LinxIpc distribution package v${VERSION}..."
echo "Using build directory: $BUILD_DIR"

# Check if library is built
if [ ! -f "$BUILD_DIR/output/lib/libLinxIpc.a" ]; then
    echo "Error: Library not found at: $BUILD_DIR/output/lib/libLinxIpc.a"
    echo "Please build the project first:"
    echo "  cmake -B $BUILD_DIR"
    echo "  cmake --build $BUILD_DIR"
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

# Copy documentation files to doc subdirectory
echo "Copying documentation..."
mkdir -p "$PACKAGE_DIR/doc"
if [ -f "$PROJECT_ROOT/README.md" ]; then
    cp "$PROJECT_ROOT/README.md" "$PACKAGE_DIR/doc/"
fi
if [ -f "$PROJECT_ROOT/doc/ARCHITECTURE.md" ]; then
    cp "$PROJECT_ROOT/doc/ARCHITECTURE.md" "$PACKAGE_DIR/doc/"
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
echo "To extract: tar -xzf ${DIST_DIR}/${PACKAGE_NAME}.tar.gz"
