.include "test_hdr.inc"

; DESC: Zero page,X addressing works

test:
    ldx #0
    lda #2
    sta $20,X
    inx
    sta $20,X
    TEST_END
