.include "test_hdr.inc"

; DESC: Basic transfer from Y to A

test:
    ldy #1
    tya
    TEST_END
