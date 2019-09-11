.include "test_hdr.inc"

; DESC: Return from subroutine works

test:
    ldx #1
    jsr routine
    inx
    jmp end
routine:
    inx
    rts
end:
    TEST_END
