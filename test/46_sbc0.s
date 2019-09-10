.include "test_hdr.inc"

; DESC: Basic subtract with carry

test:
    lda #2
    sta $2000
    lda #3
    sec
    sbc $2000
    TEST_END
