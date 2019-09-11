.include "test_hdr.inc"

; DESC: addressing mode test

test:
    lda #13
    sta $30
    lda #2
    lda $30
    sta $1230
    lda $1230
    ldx #05
    lda $30,X
    lda ($40,X)
    TEST_END

