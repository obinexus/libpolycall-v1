# PyPolyCall Installation Guide

## Installation Methods

### 1. Local Development Installation

From the libpolycall-v1trial root directory:

```bash
cd bindings/pypolycall
pip install -e .
```

### 2. Development Installation with Dependencies

```bash
cd bindings/pypolycall
pip install -e ".[dev,telemetry,crypto]"
```

### 3. Remote Installation from Git

```bash
pip install git+https://github.com/obinexus/libpolycall-v1trial.git#subdirectory=bindings/pypolycall
```

### 4. Remote Installation with Specific Branch

```bash
pip install git+https://github.com/obinexus/libpolycall-v1trial.git@main#subdirectory=bindings/pypolycall
```

## Verification

After installation, verify the binding:

```bash
python verify_installation.py
```

Or test the CLI:

```bash
pypolycall info
pypolycall test --host localhost --port 8084
```

## Protocol Requirements

**IMPORTANT**: PyPolyCall requires the polycall.exe runtime to function.

1. Ensure polycall.exe is built and available
2. Start the runtime: `polycall.exe server --port 8084`
3. Test connection: `pypolycall test`

## Troubleshooting

### Import Errors

If you encounter import errors, ensure you're installing from the correct directory:

```bash
# From project root
cd bindings/pypolycall
pip install -e .
```

### Protocol Connection Issues

1. Verify polycall.exe is running: `netstat -an | grep 8084`
2. Check firewall settings
3. Verify protocol version compatibility

### Dependencies

For missing dependencies:

```bash
pip install -r requirements.txt
```

## Development Setup

For development work:

```bash
cd bindings/pypolycall
pip install -e ".[dev]"
pytest tests/
black pypolycall/
mypy pypolycall/
```

## Support

- Issues: https://github.com/obinexus/libpolycall-v1trial/issues
- Documentation: https://docs.obinexuscomputing.com/libpolycall
