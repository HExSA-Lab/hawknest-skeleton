.include "test_hdr.inc"

; DESC: Jump to subroutine works

test:
    ldx #1
    jsr routine
    dex
routine:
    inx
    TEST_END
