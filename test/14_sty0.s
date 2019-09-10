.include "test_hdr.inc"

; DESC: Basic store from Y register works

test:
    ldy #100
    sty $2000
    TEST_END
