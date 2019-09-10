.include "test_hdr.inc"

; DESC: Set interrupt disable flag works

test:
    sei ; should set irq disable flag
    TEST_END
