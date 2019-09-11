.include "test_hdr.inc"

; DESC: Index-indirect addressing works

test:
    ldx #$1
    lda #$5
    sta $1
    lda #$7
    sta $2
    ldy #$a
    sty $0705
    lda ($00,X) ; should produce #$a in A reg
    TEST_END
