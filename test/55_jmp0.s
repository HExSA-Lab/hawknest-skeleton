.include "test_hdr.inc"

; DESC: Basic absolute jump

test:
    lda #1
    jmp target2
    tay
    iny
    iny
    iny
target2:
    nop
    TEST_END
