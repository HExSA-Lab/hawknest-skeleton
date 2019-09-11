.include "test_hdr.inc"

; DESC: Absolute,X indexing works

test:
    ldx #0
    lda #2
    sta $2000,X
    inx
    sta $2000,X
    TEST_END
