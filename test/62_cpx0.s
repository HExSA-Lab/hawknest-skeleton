.include "test_hdr.inc"

; DESC: compare X with memory location works

test:
    ldx #1
    stx $2000
    cpx $2000 ; should set zero flag
    TEST_END
