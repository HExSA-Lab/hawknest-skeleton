.include "test_hdr.inc"

; DESC: Increment with X reg works

test:
    ldx #2
    inx
    TEST_END
