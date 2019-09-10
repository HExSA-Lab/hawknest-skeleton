.include "test_hdr.inc"

; DESC: Set carry flag works

test:
    sec ; should set carry flag
    TEST_END
