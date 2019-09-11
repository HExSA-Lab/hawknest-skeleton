.include "test_hdr.inc"

; DESC: compare A with memory location works

test:
    lda #1
    sta $2000
    cmp $2000 ; should set the zero flag
    TEST_END
