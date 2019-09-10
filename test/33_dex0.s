.include "test_hdr.inc"

; DESC: Decrement with X reg works

test:
    ldx #2
    inx
    dex ; should produce 2
    TEST_END
