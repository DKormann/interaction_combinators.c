#!/usr/bin/env python3
"""
Diagnostic script for App NULL s1 issues.
Enable this when you encounter the "App node has NULL s1" error.
"""

from main import run_term_c, load_term_c, unload_term_c, to_c_data
from node import DEBUG, Node, Tag, app
from example import cnat

def check_term_validity(term):
    """Check if a term has any invalid App nodes"""
    print("=" * 60)
    print("Term Validity Check")
    print("=" * 60)
    
    try:
        data = to_c_data(term)
        print("✓ Python serialization validation passed")
        print(f"  Total serialized data length: {len(data)}")
        print(f"  Number of nodes: {data[0]}")
    except ValueError as e:
        print(f"✗ Python serialization FAILED:")
        print(f"  {e}")
        return False
    
    # Parse and display serialization
    print("\nSerialized structure:")
    count = data[0]
    tag_names = {0: "App", 1: "Lam", 2: "Sup", 3: "Dup", 4: "Dup2", 5: "Null", 6: "Var"}
    
    for i in range(count):
        idx = i * 4 + 1
        tag = data[idx]
        label = data[idx + 1]
        s0 = data[idx + 2]
        s1 = data[idx + 3]
        tag_name = tag_names.get(tag, "UNK")
        
        s0_str = f"node[{s0}]" if s0 > 0 else "NULL"
        s1_str = f"node[{s1}]" if s1 > 0 else "NULL"
        
        if tag_name == "App" and s1 == 0:
            print(f"  [{i+1}] {tag_name:4s} label={label} s0={s0_str} s1={s1_str} ❌ INVALID!")
        else:
            print(f"  [{i+1}] {tag_name:4s} label={label} s0={s0_str} s1={s1_str}")
    
    return True

def test_with_debug(term):
    """Test a term with debug output enabled"""
    print("\n" + "=" * 60)
    print("Running with DEBUG output")
    print("=" * 60)
    
    DEBUG.set(True)
    try:
        result = run_term_c(term, steps=1000)
        print(f"\n✓ Reduction succeeded")
        return True
    except Exception as e:
        print(f"\n✗ Reduction failed: {e}")
        return False
    finally:
        DEBUG.set(False)

if __name__ == "__main__":
    # Test 1: Known good term
    print("\nTest 1: Known good term cnat(1)")
    print("-" * 60)
    term1 = cnat(1)
    check_term_validity(term1)
    
    # Test 2: More complex term
    print("\n\nTest 2: Known good term cnat(2)")
    print("-" * 60)
    term2 = cnat(2)
    check_term_validity(term2)
    
    # Test 3: Custom constructed app
    print("\n\nTest 3: Custom app construction")
    print("-" * 60)
    identity = Node(lambda x: x)
    arg = Node(None)
    arg.tag = Tag.Null
    custom_app = app(identity, arg)
    check_term_validity(custom_app)
    
    # Test 4: Run with debug
    print("\n\nTest 4: Running cnat(1) with debug enabled")
    test_with_debug(cnat(1))
