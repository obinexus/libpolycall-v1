#!/bin/bash
# analyze_polycall.sh - Analyze and map the PolyCall directory structure

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo -e "${GREEN}=== PolyCall Structure Analyzer ===${NC}"
echo "Current directory: $(pwd)"
echo ""

# Function to find all C source files
find_sources() {
    echo -e "${YELLOW}Searching for C source files...${NC}"
    
    # Find in current directory
    if [[ -d "src" ]]; then
        echo -e "${BLUE}Found src/ in current directory:${NC}"
        find src -name "*.c" -type f 2>/dev/null | head -20
    fi
    
    # Find in v2
    if [[ -d "v2/src" ]]; then
        echo -e "${BLUE}Found v2/src/:${NC}"
        find v2/src -name "*.c" -type f 2>/dev/null | head -20
    fi
    
    # Find in libpolycall
    if [[ -d "libpolycall/src" ]]; then
        echo -e "${BLUE}Found libpolycall/src/:${NC}"
        find libpolycall/src -name "*.c" -type f 2>/dev/null | head -20
    fi
    
    # Find in polycall-v2
    if [[ -d "v2/polycall-v2" ]]; then
        echo -e "${BLUE}Found v2/polycall-v2/:${NC}"
        find v2/polycall-v2 -name "*.c" -type f 2>/dev/null | head -20
    fi
    
    # Find in any extracted directories
    if [[ -d "polycall_extracted" ]]; then
        echo -e "${BLUE}Found polycall_extracted/:${NC}"
        find polycall_extracted -name "*.c" -type f 2>/dev/null | head -20
    fi
}

# Function to find all header files
find_headers() {
    echo -e "${YELLOW}Searching for header files...${NC}"
    
    for dir in . v2 libpolycall v2/polycall-v2; do
        if [[ -d "$dir/include" ]]; then
            echo -e "${BLUE}Found $dir/include/:${NC}"
            find "$dir/include" -name "*.h" -type f 2>/dev/null | head -10
        fi
    done
}

# Function to check for build files
check_build_files() {
    echo -e "${YELLOW}Checking for build files...${NC}"
    
    # Check for Makefiles
    echo -e "${MAGENTA}Makefiles:${NC}"
    find . -name "Makefile" -o -name "makefile" -o -name "*.mk" 2>/dev/null | grep -v ".git"
    
    # Check for CMake files
    echo -e "${MAGENTA}CMake files:${NC}"
    find . -name "CMakeLists.txt*" 2>/dev/null | grep -v ".git"
    
    # Check for existing libraries
    echo -e "${MAGENTA}Existing libraries:${NC}"
    find . -name "*.a" -o -name "*.so" -o -name "*.so.*" 2>/dev/null | grep -v ".git"
}

# Function to check archive files
check_archives() {
    echo -e "${YELLOW}Checking for archive files...${NC}"
    
    if [[ -f "polycall.rar" ]]; then
        echo -e "${GREEN}Found polycall.rar${NC}"
        echo "Size: $(du -h polycall.rar | cut -f1)"
        
        if command -v unrar &> /dev/null; then
            echo "Contents preview:"
            unrar l polycall.rar 2>/dev/null | head -20
        else
            echo -e "${RED}unrar not installed - cannot list contents${NC}"
            echo "Install with: apt-get install unrar"
        fi
    fi
    
    # Check for other archives
    for archive in *.tar *.tar.gz *.zip *.tar.bz2; do
        if [[ -f "$archive" ]]; then
            echo "Found archive: $archive ($(du -h "$archive" | cut -f1))"
        fi
    done
}

# Function to map v2 structure specifically
map_v2_structure() {
    if [[ ! -d "v2" ]]; then
        return
    fi
    
    echo -e "${YELLOW}=== V2 Directory Structure ===${NC}"
    cd v2
    
    echo -e "${BLUE}Main directories in v2/:${NC}"
    ls -la --color=auto | grep "^d"
    
    echo ""
    echo -e "${BLUE}Contents of key directories:${NC}"
    
    # Check isolated bindings
    if [[ -d "isolated/bindings" ]]; then
        echo "isolated/bindings structure:"
        tree -L 3 isolated/bindings 2>/dev/null || ls -R isolated/bindings | head -30
    fi
    
    # Check extensions
    if [[ -d "extensions" ]]; then
        echo "extensions:"
        ls -la extensions/
    fi
    
    # Check polycall-v2
    if [[ -d "polycall-v2" ]]; then
        echo "polycall-v2 structure:"
        tree -L 2 polycall-v2 2>/dev/null || ls -la polycall-v2/
    fi
    
    cd ..
}

# Main analysis
echo -e "${GREEN}=== Starting Analysis ===${NC}"
echo ""

find_sources
echo ""

find_headers
echo ""

check_build_files
echo ""

check_archives
echo ""

map_v2_structure
echo ""

# Summary and recommendations
echo -e "${GREEN}=== Analysis Summary ===${NC}"
echo "To repair the build structure:"
echo ""

if [[ -f "polycall.rar" ]] && ! [[ -d "polycall_extracted" ]]; then
    echo "1. Extract polycall.rar:"
    echo "   unrar x polycall.rar"
    echo ""
fi

if [[ -d "v2" ]]; then
    echo "2. Use the v2 repair script:"
    echo "   cd v2"
    echo "   chmod +x ../polycall_repair_v2.sh"
    echo "   ../polycall_repair_v2.sh"
else
    echo "2. Run repair from this directory:"
    echo "   chmod +x polycall_repair_v2.sh"
    echo "   ./polycall_repair_v2.sh"
fi

echo ""
echo -e "${BLUE}OBINexus Project Status:${NC}"
echo "- Toolchain: riftlang.exe → .so.a → rift.exe → gosilang"
echo "- Build: nlink → polybuild"
echo "- Current phase: Module structure repair"
