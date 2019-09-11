.include "test_hdr.inc"

; DESC: Test if byte is alpha character

; checks to see if byte in A is an alphabetical
; character. if so, puts 1 in X, else 0

isalpha:
    cmp #'A'
    bcc fail
    cmp #'Z' + 1
    bcc pass
    cmp #'a'
    bcc fail
    cmp #'z' + 1
    bcs fail
pass:
    ldx #1
    rts
fail:
    ldx #0
    rts
       
test:
    lda #127
    jsr isalpha
    TEST_END

