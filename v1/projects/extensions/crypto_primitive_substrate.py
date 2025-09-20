#!/usr/bin/env python3
"""
OBINexus Crypto Primitive Substrate
Secure rehash implementation with HKDF + dimensional architecture
Philosophy: Sue Sue Sue - A Trilogy to Live By
  1. Sue for Security (cryptographic assurance)
  2. Sue for Stability (operational resilience)  
  3. Sue for Sovereignty (data control & rotation)
"""

import hmac
import hashlib
import secrets
from typing import Union, Tuple, Optional, Dict
from dataclasses import dataclass
from enum import Enum
import binascii
import time

# ---------- OBINexus Enums ----------
class UpdateMode(Enum):
    ATOMIC = "atomic"
    BATCHED = "batched"
    STAGED = "staged"

class ExposureLevel(Enum):
    CONTROLLED = "controlled"
    STAGED = "staged"
    FULL = "full"
    ZERO_KNOWLEDGE = "zero_knowledge"

class DimensionLayer(Enum):
    SUBSTRATE_0 = "substrate_0"  # Maximum security primitives
    SUBSTRATE_1 = "substrate_1"  # Staged deployment primitives
    SUBSTRATE_2 = "substrate_2"  # Application layer

# ---------- Data Classes ----------
@dataclass
class SubstrateState:
    """State representation for substrate operations"""
    primitive: bytes
    dimension: DimensionLayer
    exposure_level: ExposureLevel
    timestamp: float
    rotation_counter: int

@dataclass 
class DeploymentResult:
    """Result of production deployment"""
    success: bool
    substrate_state: SubstrateState
    exposure_level: str
    derived_key: Optional[str] = None

# ---------- Utility Functions ----------
def to_bytes(x: Union[str, bytes], *, allow_hex: bool = True) -> bytes:
    """Safely convert input to bytes"""
    if isinstance(x, bytes):
        return x
    if allow_hex:
        try:
            # Handle hex with or without 0x prefix
            if x.startswith(('#', '0x', '0X')):
                x = x[2:] if x.startswith('0x') or x.startswith('0X') else x[1:]
            return bytes.fromhex(x)
        except Exception:
            pass
    return x.encode("utf-8")

def hexify(b: bytes) -> str:
    """Convert bytes to hex string"""
    return binascii.hexlify(b).decode("ascii")

def constant_time_compare(a: bytes, b: bytes) -> bool:
    """Constant-time comparison to prevent timing attacks"""
    return hmac.compare_digest(a, b)

# ---------- HKDF Implementation (RFC 5869) ----------
def hkdf_extract(salt: bytes, ikm: bytes, hashmod=hashlib.sha256) -> bytes:
    """HKDF-Extract: PRK = HMAC-Hash(salt, IKM)"""
    if salt is None or len(salt) == 0:
        salt = b"\x00" * hashmod().digest_size
    return hmac.new(salt, ikm, hashmod).digest()

def hkdf_expand(prk: bytes, info: bytes, length: int, hashmod=hashlib.sha256) -> bytes:
    """HKDF-Expand: Generate output key material"""
    hash_len = hashmod().digest_size
    if length > 255 * hash_len:
        raise ValueError("Cannot expand to more than 255 * HashLen bytes")
    
    okm = b""
    t = b""
    counter = 1
    
    while len(okm) < length:
        t = hmac.new(prk, t + info + bytes([counter]), hashmod).digest()
        okm += t
        counter += 1
    
    return okm[:length]

# ---------- Core Rehash Function ----------
def rehash(salt: Union[str, bytes],
           pub_key: Union[str, bytes], 
           priv_key: Union[str, bytes],
           out_len: int = 32,
           context: Optional[str] = None) -> str:
    """
    OBINexus secure key derivation using HKDF-SHA256
    
    Args:
        salt: Rotation salt (public), supports hex/UTF-8
        pub_key: Public key/context info 
        priv_key: Private key (secret) - MUST be protected
        out_len: Output length in bytes (default 32)
        context: Optional additional context (e.g., "password_rotation_v1")
    
    Returns:
        Hex string of derived key
    """
    # Convert inputs to bytes
    salt_b = to_bytes(salt)
    pub_b = to_bytes(pub_key, allow_hex=True)
    priv_b = to_bytes(priv_key, allow_hex=True)
    
    # Add context if provided
    if context:
        pub_b = pub_b + b"|" + context.encode("utf-8")
    
    # HKDF Extract phase
    prk = hkdf_extract(salt_b, priv_b)
    
    # HKDF Expand phase with public key as info
    okm = hkdf_expand(prk, pub_b, out_len)
    
    return hexify(okm)

# ---------- Substrate Management ----------
class CryptoPrimitiveSubstrate:
    """
    OBINexus substrate dimension for cryptographic operations
    Implements the "Sue Sue Sue" philosophy
    """
    
    def __init__(self, dimension: DimensionLayer = DimensionLayer.SUBSTRATE_0):
        self.dimension = dimension
        self.update_mode = UpdateMode.ATOMIC
        self.exposure_level = ExposureLevel.CONTROLLED
        self.rotation_counter = 0
        self.substrate_key = secrets.token_bytes(32)
        self.pentext_salt = secrets.token_bytes(16)
        
    def substrate_update(self, primitive_data: bytes) -> SubstrateState:
        """Atomic update of cryptographic primitives"""
        # Verify substrate dimension
        if not self._verify_substrate_dimension():
            raise ValueError("Invalid substrate dimension")
        
        # Atomic primitive update
        updated_primitive = self._atomic_primitive_update(primitive_data)
        
        # Production exposure control
        exposed_primitive = self._controlled_exposure(updated_primitive)
        
        self.rotation_counter += 1
        
        return SubstrateState(
            primitive=exposed_primitive,
            dimension=self.dimension,
            exposure_level=self.exposure_level,
            timestamp=time.time(),
            rotation_counter=self.rotation_counter
        )
    
    def rotate_key(self, pub_key: Union[str, bytes], 
                   priv_key: Union[str, bytes]) -> DeploymentResult:
        """
        Implement key rotation using rehash
        Sue #1: Security through proper key derivation
        """
        # Generate new salt for rotation
        new_salt = secrets.token_hex(16)
        
        # Derive new key
        derived_key = rehash(
            salt=new_salt,
            pub_key=pub_key,
            priv_key=priv_key,
            out_len=32,
            context=f"obinexus_rotation_{self.rotation_counter}"
        )
        
        # Update substrate state
        state = self.substrate_update(bytes.fromhex(derived_key))
        
        return DeploymentResult(
            success=True,
            substrate_state=state,
            exposure_level=self.exposure_level.value,
            derived_key=derived_key
        )
    
    def _verify_substrate_dimension(self) -> bool:
        """Verify we're operating in correct dimension"""
        return self.dimension in DimensionLayer
    
    def _atomic_primitive_update(self, data: bytes) -> bytes:
        """
        Sue #2: Stability through atomic updates
        """
        # Ensure atomic operation
        return hmac.new(self.substrate_key, data, hashlib.sha256).digest()
    
    def _controlled_exposure(self, primitive: bytes) -> bytes:
        """
        Sue #3: Sovereignty through controlled exposure
        """
        if self.exposure_level == ExposureLevel.ZERO_KNOWLEDGE:
            # Return commitment only
            return hashlib.sha256(primitive).digest()
        elif self.exposure_level == ExposureLevel.CONTROLLED:
            # Return masked version
            mask = hmac.new(self.pentext_salt, b"mask", hashlib.sha256).digest()
            return bytes(a ^ b for a, b in zip(primitive, mask))
        else:
            return primitive
    
    def pentext_channel_security(self, data: bytes) -> bytes:
        """Maximum security channel for production deployment"""
        return hashlib.blake2b(
            data,
            digest_size=64,  # 512-bit security
            key=self.substrate_key,
            salt=self.pentext_salt
        ).digest()

# ---------- Testing Framework ----------
class SubstrateTestHarness:
    """Test harness for substrate operations"""
    
    @staticmethod
    def collision_test(trials: int = 10000, out_len: int = 32) -> Tuple[int, int]:
        """Test for collisions in derived keys"""
        seen = {}
        collisions = 0
        
        # Fixed keys for testing
        priv = secrets.token_bytes(32)
        pub = secrets.token_bytes(32)
        
        for i in range(trials):
            salt = secrets.token_bytes(16)
            dk = rehash(salt, pub, priv, out_len=out_len)
            
            if dk in seen:
                collisions += 1
            else:
                seen[dk] = i
        
        return trials, collisions
    
    @staticmethod
    def fault_injection_test(trials: int = 10) -> Dict[str, int]:
        """Test robustness against bit flips"""
        results = {"salt_flips": 0, "pub_flips": 0, "priv_flips": 0}
        
        priv = secrets.token_bytes(32)
        pub = secrets.token_bytes(32) 
        salt = secrets.token_bytes(16)
        
        base = rehash(salt, pub, priv, out_len=32)
        
        for _ in range(trials):
            # Test each component
            for target in ["salt", "pub", "priv"]:
                if target == "salt":
                    test_val = bytearray(salt)
                elif target == "pub":
                    test_val = bytearray(pub)
                else:
                    test_val = bytearray(priv)
                
                # Flip random bit
                idx = secrets.randbelow(len(test_val))
                test_val[idx] ^= (1 << secrets.randbelow(8))
                
                # Compute new hash
                if target == "salt":
                    new = rehash(bytes(test_val), pub, priv)
                elif target == "pub":
                    new = rehash(salt, bytes(test_val), priv)
                else:
                    new = rehash(salt, pub, bytes(test_val))
                
                # Check avalanche effect
                if new != base:
                    results[f"{target}_flips"] += 1
        
        return results
    
    @staticmethod
    def timing_analysis(iterations: int = 1000) -> float:
        """Basic timing analysis for side-channel awareness"""
        priv = secrets.token_bytes(32)
        pub = secrets.token_bytes(32)
        
        timings = []
        for _ in range(iterations):
            salt = secrets.token_bytes(16)
            
            start = time.perf_counter()
            _ = rehash(salt, pub, priv)
            end = time.perf_counter()
            
            timings.append(end - start)
        
        avg_time = sum(timings) / len(timings)
        variance = sum((t - avg_time) ** 2 for t in timings) / len(timings)
        
        return variance  # Lower is better for timing attack resistance

# ---------- Example Usage ----------
if __name__ == "__main__":
    print("=== OBINexus Crypto Primitive Substrate ===")
    print("Philosophy: Sue Sue Sue - A Trilogy to Live By\n")
    
    # Initialize substrate
    substrate = CryptoPrimitiveSubstrate(DimensionLayer.SUBSTRATE_0)
    
    # Example key rotation
    pub_key = "1ABITONE"
    priv_key = "12859"  # In production, use secure random bytes
    
    print("1. Initial key derivation:")
    salt = "A82758"
    derived = rehash(salt, pub_key, priv_key)
    print(f"   Derived key: {derived}")
    
    print("\n2. Key rotation (Sue #1: Security):")
    result = substrate.rotate_key(pub_key, priv_key)
    print(f"   Success: {result.success}")
    print(f"   New key: {result.derived_key}")
    print(f"   Rotation count: {result.substrate_state.rotation_counter}")
    
    print("\n3. Collision testing (Sue #2: Stability):")
    trials, collisions = SubstrateTestHarness.collision_test(trials=5000)
    print(f"   Tested {trials} keys, collisions: {collisions}")
    
    print("\n4. Fault injection (Sue #3: Sovereignty):")
    fault_results = SubstrateTestHarness.fault_injection_test(trials=10)
    print(f"   Bit flip detection rates: {fault_results}")
    
    print("\n5. Timing analysis:")
    variance = SubstrateTestHarness.timing_analysis(iterations=100)
    print(f"   Timing variance: {variance:.2e} (lower is better)")
    
    print("\n[OBINexus substrate initialized successfully]")
