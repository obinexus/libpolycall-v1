# test_separation.py - Verify NO business logic in core
def test_core_has_no_business_logic():
    """Core should only transport, never decide"""
    
    # This should work - pure transport
    result = libpolycall.dispatch({
        "service": "banking",
        "action": "withdraw",
        "data": {"amount": 100}
    })
    assert result.status == "dispatched"
    
    # This should NOT exist in core
    with pytest.raises(AttributeError):
        libpolycall.validate_withdrawal()  # Business logic!
