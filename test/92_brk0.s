.include "test_hdr_novecs.inc"

; DESC: break instruction (software interrupt) works

.segment "VECTORS"
    .word 0
    .word test
	.word irq

.segment "CODE"

handle_irq:
    lda #1
    nop
    nop
    nop
    nop
    rts

irq: 
    ; save regs
    pha
    tya
    pha
    txa
    pha

    jsr handle_irq

    ; restore regs
    pla
    tax
    pla
    tay
    pla

    rti ;; return from interrupt

test:
    nop
    nop
    brk
    nop
    nop
    TEST_END

