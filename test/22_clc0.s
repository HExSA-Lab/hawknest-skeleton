.include "test_hdr.inc"

; DESC: Clear carry flag works

test:
    sec ; should set carry flag
    clc ; should then clear it
    TEST_END
