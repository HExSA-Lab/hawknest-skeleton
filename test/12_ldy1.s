.include "test_hdr.inc"

; DESC: Absolute,Y indexing works

test:
    lda #200
    ldy #1
    sta $2000, Y ;; should store the value 200 at address 0x2001
    TEST_END
