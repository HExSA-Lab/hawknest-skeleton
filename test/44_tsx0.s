.include "test_hdr.inc"

; DESC: Put stack pointer into X

test:
    tsx ; X should have $ff now
    TEST_END
