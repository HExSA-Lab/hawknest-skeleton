.include "test_hdr.inc"

; DESC: find max element in array

maxm:
    lda #0
cmpe:
    dey
    php
    cmp ($40),Y
    bcs no_chg
    lda ($40),Y
no_chg:
    plp
    bne cmpe
    rts

find_max:
    ldx #$ff
    txs
    lda #$43
    sta $40
    lda #0
    sta $41
    ldy $30
    jsr maxm
    sta $42

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

