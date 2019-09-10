.include "test_hdr.inc"

; DESC: Absolute,X indexing test

test:
    ldx #1
    lda #200
    sta $2000, X ;; should store the value 200 at address $0x2001
    TEST_END
