.include "test_hdr.inc"

; DESC: Branch if carry clear works (not taken condition)

test:
    ldy #1
    sec              ;; carry is set
    bcc add_another
    jmp dont
add_another:
    iny
dont:
    nop
    TEST_END
