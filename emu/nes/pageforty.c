#include <rc.h>
#include <nes/io_reg.h>
#include <nes/apu.h>

typedef struct pageforty {
	io_reg_t * nonnull /*strong*/ io;
	apu_t * nonnull /*strong*/ apu;
} pageforty_t;

static void
deinit (pageforty_t * nonnull pageforty)
{
	rc_release(pageforty->io);
	rc_release(pageforty->apu);
}

static uint8_t
read (pageforty_t * pageforty, uint16_t addr, uint8_t * lanemask)
{
	if (addr == 0x16 || addr == 0x17) {
		return io_reg_mem_read(pageforty->io, addr, lanemask);
	} else {
		return apu_mem_read(pageforty->apu, addr, lanemask);
	}
}

static void
write (pageforty_t * pageforty, uint16_t addr, uint8_t val)
{
	if (addr == 0x14 || addr == 0x16) {
		io_reg_mem_write(pageforty->io, addr, val);
	} else {
		apu_mem_write(pageforty->apu, addr, val);
	}
}

int
pageforty_setup (reset_manager_t * rm, mos6502_t * cpu, const char * cscheme_path)
{
	int retcode = -1;

	io_reg_t * io = io_reg_new(rm, cpu, cscheme_path);
	if (!io) {
		ERROR_PRINT("Couldn't create an IO register device");
		goto ret;
	}

	apu_t * apu = apu_new(rm, cpu);
	if (!apu) {
		ERROR_PRINT("Couldn't create an APU device");
		rc_release(io);
		goto ret;
	}

	pageforty_t *pageforty = rc_alloc(sizeof(pageforty_t), deinit);
	pageforty->io = io;
	pageforty->apu = apu;

	membus_set_read_handler(cpu->bus, 0x40, pageforty, 0, read);
	membus_set_write_handler(cpu->bus, 0x40, pageforty, 0, write);

	retcode = 0;
	rc_release(pageforty);

ret:
	return retcode;
}
