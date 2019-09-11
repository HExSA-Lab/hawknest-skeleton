.include "test_hdr.inc"

; DESC: Indirect-index addressing works

test:
    ldy #$1
    lda #$3
    sta $1
    lda #$7
    sta $2
    ldx #$a
    stx $0704
    lda ($1),Y  ;; should produce #$a in A reg
    TEST_END
