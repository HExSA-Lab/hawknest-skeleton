.include "test_hdr.inc"

; DESC: Indirect jump with NOP sled


test:
    lda #18
    sta $2000
    lda #$a0
    sta $2001
    jmp ($2000)
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    TEST_END
