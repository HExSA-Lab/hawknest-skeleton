.include "test_hdr.inc"

; DESC: compare X with memory location works (failed comparison)

test:
    ldx #1
    stx $2000
    ldx #3
    cpx $2000 ; should *not* set zero flag
    TEST_END
