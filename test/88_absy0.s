.include "test_hdr.inc"

; DESC: Absolute,Y addressing works

test:
    ldy #1
    lda #5
    sta $0300, Y ;; should store at 1 at $301
    TEST_END
