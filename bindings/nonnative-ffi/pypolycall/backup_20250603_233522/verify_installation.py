#!/usr/bin/env python3
"""
PyPolyCall Installation Verification
Verify the binding is properly installed and can connect to polycall.exe
"""

import sys
import os
import asyncio
from pathlib import Path

def verify_package_structure():
    """Verify package structure is correct"""
    print("🔍 Verifying package structure...")
    
    required_modules = [
        'pypolycall',
        'pypolycall.core',
        'pypolycall.protocol',
        'pypolycall.config',
    ]
    
    for module in required_modules:
        try:
            __import__(module)
            print(f"  ✅ {module}")
        except ImportError as e:
            print(f"  ❌ {module}: {e}")
            return False
    
    return True

def verify_protocol_compliance():
    """Verify protocol compliance"""
    print("🔍 Verifying protocol compliance...")
    
    try:
        from pypolycall import get_protocol_info
        info = get_protocol_info()
        
        required_compliance = {
            'polycall_runtime_required': True,
            'adapter_pattern': True,
            'zero_trust_compliant': True,
        }
        
        for key, expected in required_compliance.items():
            if info.get(key) != expected:
                print(f"  ❌ Protocol compliance failed: {key}")
                return False
            print(f"  ✅ {key}: {info[key]}")
        
        return True
    except Exception as e:
        print(f"  ❌ Protocol compliance check failed: {e}")
        return False

async def verify_runtime_connection():
    """Verify can attempt connection to polycall.exe"""
    print("🔍 Verifying runtime connection capability...")
    
    try:
        from pypolycall.core import ProtocolBinding
        
        binding = ProtocolBinding(
            polycall_host="localhost",
            polycall_port=8084
        )
        
        print("  ✅ ProtocolBinding created successfully")
        print("  ℹ️  Note: Actual connection requires polycall.exe runtime")
        
        return True
    except Exception as e:
        print(f"  ❌ Runtime connection test failed: {e}")
        return False

def main():
    """Main verification function"""
    print("PyPolyCall Installation Verification")
    print("=" * 40)
    
    tests = [
        verify_package_structure,
        verify_protocol_compliance,
    ]
    
    results = []
    for test in tests:
        try:
            if asyncio.iscoroutinefunction(test):
                result = asyncio.run(test())
            else:
                result = test()
            results.append(result)
        except Exception as e:
            print(f"❌ Test {test.__name__} failed with exception: {e}")
            results.append(False)
        print()
    
    # Run async test separately
    try:
        async_result = asyncio.run(verify_runtime_connection())
        results.append(async_result)
    except Exception as e:
        print(f"❌ Async test failed: {e}")
        results.append(False)
    
    print("=" * 40)
    if all(results):
        print("🎉 All verification tests passed!")
        print("PyPolyCall binding is properly installed")
        return 0
    else:
        print("❌ Some verification tests failed")
        print("Please check the installation and try again")
        return 1

if __name__ == "__main__":
    sys.exit(main())
