.include "test_hdr.inc"

; DESC: Store to zero page from Y register works

test:
    ldy #100
    sty $20
    TEST_END
