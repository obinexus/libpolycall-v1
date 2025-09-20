#!/bin/bash

# Simple Setup.py Consolidation Script
# Works from current PyPolyCall directory
# Author: OBINexusComputing

set -euo pipefail

echo "[TECHNICAL] Simple setup.py consolidation for PyPolyCall"
echo "[TECHNICAL] Working directory: $(pwd)"

# Check if we're in the right place
if [[ ! -d "pypolycall" ]]; then
    echo "[ERROR] pypolycall directory not found in current location"
    echo "[ERROR] Please run this script from the PyPolyCall binding directory"
    exit 1
fi

if [[ ! -f "setup.py" ]]; then
    echo "[WARNING] No setup.py found - creating new one"
fi

echo "[TECHNICAL] Creating backup..."
backup_dir="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "${backup_dir}"
cp -r . "${backup_dir}/" 2>/dev/null || true
echo "[SUCCESS] Backup created: ${backup_dir}"

echo "[TECHNICAL] Removing duplicate setup.py files..."
find . -name "setup.py" -not -path "./setup.py" -delete 2>/dev/null || true

echo "[TECHNICAL] Creating canonical setup.py..."
cat > setup.py << 'EOF'
#!/usr/bin/env python3
"""
PyPolyCall - LibPolyCall Trial v1 Python Binding
Protocol-compliant adapter for polycall.exe runtime
"""

from setuptools import setup, find_packages
import os
import sys

# Version compatibility check
if sys.version_info < (3, 8):
    raise RuntimeError("PyPolyCall requires Python 3.8 or higher")

# Read README
def read_readme():
    readme_path = os.path.join(os.path.dirname(__file__), 'README.md')
    if os.path.exists(readme_path):
        with open(readme_path, 'r', encoding='utf-8') as f:
            return f.read()
    return "PyPolyCall - LibPolyCall Trial v1 Python Binding"

# Read requirements
def read_requirements():
    req_path = os.path.join(os.path.dirname(__file__), 'requirements.txt')
    requirements = []
    
    if os.path.exists(req_path):
        with open(req_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):
                    requirements.append(line)
    
    # Fallback requirements if file doesn't exist
    if not requirements:
        requirements = [
            "aiohttp>=3.8.0",
            "websockets>=11.0.0",
            "httpx>=0.24.0",
            "msgpack>=1.0.0",
            "pydantic>=2.0.0",
            "cryptography>=41.0.0",
            "PyYAML>=6.0.0",
            "structlog>=23.0.0",
        ]
    
    return requirements

setup(
    name="pypolycall",
    version="1.0.0",
    author="OBINexusComputing",
    author_email="contact@obinexuscomputing.com",
    description="LibPolyCall Trial v1 Python Binding - Protocol-compliant adapter",
    long_description=read_readme(),
    long_description_content_type="text/markdown",
    url="https://gitlab.com/obinexuscomputing/libpolycall-v1trial/tree/main/bindings/pypolycall",
    packages=find_packages(exclude=["tests", "tests.*", "docs", "examples"]),
    python_requires=">=3.8",
    install_requires=read_requirements(),
    extras_require={
        "dev": [
            "pytest>=7.0.0",
            "pytest-cov>=4.0.0",
            "pytest-asyncio>=0.21.0",
            "black>=23.0.0",
            "flake8>=6.0.0",
            "mypy>=1.0.0",
            "isort>=5.12.0",
        ],
        "telemetry": [
            "prometheus-client>=0.17.0",
            "structlog>=23.0.0",
        ],
        "crypto": [
            "cryptography>=41.0.0",
            "pycryptodome>=3.18.0",
        ],
    },
    entry_points={
        "console_scripts": [
            "pypolycall=pypolycall.cli.main:main",
        ],
    },
    project_urls={
        "Bug Reports": "https://gitlab.com/obinexuscomputing/libpolycall-v1trial/-/issues",
        "Source": "https://gitlab.com/obinexuscomputing/libpolycall-v1trial/tree/main/bindings/pypolycall",
        "Documentation": "https://docs.obinexuscomputing.com/libpolycall",
    },
    keywords="api binding protocol polymorphic zero-trust telemetry",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: System :: Networking",
        "Topic :: Internet :: WWW/HTTP :: HTTP Servers",
    ],
    zip_safe=False,
    include_package_data=True,
    package_data={
        "pypolycall": [
            "config/*.json",
            "config/*.yaml",
            "templates/*.py",
        ],
    },
)
EOF

echo "[TECHNICAL] Cleaning build artifacts..."
rm -rf build/ dist/ *.egg-info/ .eggs/ 2>/dev/null || true
find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
find . -name "*.pyc" -delete 2>/dev/null || true

echo "[TECHNICAL] Testing setup.py validity..."
python setup.py check --strict --metadata || {
    echo "[ERROR] Setup.py validation failed"
    exit 1
}

echo "[TECHNICAL] Testing installation..."
pip uninstall -y pypolycall 2>/dev/null || true
pip install -e . || {
    echo "[ERROR] Installation failed"
    exit 1
}

echo "[TECHNICAL] Verifying package import..."
python -c "
import pypolycall
print(f'✓ PyPolyCall version: {pypolycall.__version__}')
print(f'✓ Package location: {pypolycall.__file__}')
print('✓ Import successful')
" || {
    echo "[ERROR] Package import failed"
    exit 1
}

echo "[TECHNICAL] Testing CLI entry point..."
pypolycall --version || {
    echo "[ERROR] CLI entry point failed"
    exit 1
}

echo "[SUCCESS] Setup.py consolidation completed successfully!"
echo ""
echo "Technical Results:"
echo "✓ Single canonical setup.py created"
echo "✓ Package structure validated"
echo "✓ Installation testing passed"
echo "✓ CLI entry point functional"
echo ""
echo "Next steps:"
echo "1. pytest tests/ -v"
echo "2. pypolycall info"
echo "3. python -c 'import pypolycall; print(pypolycall.__version__)'"
echo ""
echo "Backup available in: ${backup_dir}"
