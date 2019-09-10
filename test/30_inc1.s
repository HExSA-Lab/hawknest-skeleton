.include "test_hdr.inc"

; DESC: Basic increment works with zero page

test:
    lda #2
    sta $20
    inc $20
    TEST_END
