.include "test_hdr.inc"

; DESC: Decrement with Y reg works

test:
    ldy #2
    iny
    dey ; should produce 2
    TEST_END
