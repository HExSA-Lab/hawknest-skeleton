.include "test_hdr.inc"

; DESC: compare A with immediate works

test:
    lda #1
    cmp #1 ; should set the zero flag
    TEST_END
