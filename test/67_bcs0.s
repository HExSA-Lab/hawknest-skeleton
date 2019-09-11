.include "test_hdr.inc"

; DESC: Branch if carry set works (taken condition)

test:
    ldy #1
    sec              ;; carry is set
    bcs add_another  ;; should be taken
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
