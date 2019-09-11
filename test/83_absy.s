.include "test_hdr.inc"

; DESC: Absolute,Y indexing works

test:
    ldy #0
    lda #2
    sta $2000,Y
    iny
    sta $2000,Y
    TEST_END
