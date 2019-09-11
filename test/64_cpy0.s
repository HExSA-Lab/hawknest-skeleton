.include "test_hdr.inc"

; DESC: compare Y with memory location works 

test:
    ldy #1
    sty $2000
    cpy $2000 ; should set zero flag
    TEST_END
