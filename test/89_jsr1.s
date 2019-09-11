.include "test_hdr.inc"

; DESC: multiple subroutine jumps with looping

test:
    jsr init
    jsr loop
    jsr end
init:
    ldx #1
    rts
loop:
    inx
    cpx #5
    bne loop
    rts
end:
    nop
    TEST_END
