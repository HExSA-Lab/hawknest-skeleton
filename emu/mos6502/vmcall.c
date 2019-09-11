#include <base.h>
#include <membus.h>
#include <mos6502/vmcall.h>

#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

static inline uint16_t
get_ax (mos6502_t * cpu)
{
	uint16_t ax = (uint16_t)cpu->a | (uint16_t)((uint16_t)cpu->x << 8);
	return ax;
}

static inline void
set_ax (mos6502_t * cpu, uint16_t val)
{
	cpu->a = (uint8_t)(val & 0xff);
	cpu->x = (uint8_t)(val >> 8);
}

static inline uint16_t
read_zp16 (mos6502_t * cpu, uint8_t addr)
{

	return membus_read(cpu->bus, addr) | (uint16_t)(membus_read(cpu->bus, addr + 1) << 8);
}

static inline void
write16 (mos6502_t * cpu, uint16_t addr, uint16_t val)
{
	membus_write(cpu->bus, addr, (uint8_t)(val & 0xff));
	membus_write(cpu->bus, addr + 1, (uint8_t)(val >> 8));
}

// Parmesan?
static inline uint16_t
pop_parm (mos6502_t * cpu, uint16_t incr)
{

	uint16_t sp = read_zp16(cpu, 0x00);
	uint16_t val = membus_read(cpu->bus, sp) | (uint16_t)(membus_read(cpu->bus, sp + 1) << 8u);
	write16(cpu, 0x0000, sp + incr);
	return val;
}

static inline mos6502_step_result_t
handle_args (mos6502_t * cpu)
{
	uint16_t argv = get_ax(cpu);
	uint16_t sp = read_zp16(cpu, 0x00);
	uint16_t args = sp - (cpu->paravirt_argc + 1) * 2;
	uint16_t i = 0;

	write16(cpu, argv, args);

	sp = args;

	while (i < cpu->paravirt_argc) {
		uint16_t spi = 0;
		const char * thearg = cpu->paravirt_argv[i++];
		sp -= strlen(thearg) + 1;
		do {
			membus_write(cpu->bus, sp + spi, (uint8_t)thearg[spi]);
		} while (thearg[spi++]);

		write16(cpu, args, sp);
		args += 2;
	}

	write16(cpu, 0x0000, sp);
	set_ax(cpu, cpu->paravirt_argc);

	return MOS6502_STEP_RESULT_SUCCESS;
}

static inline mos6502_step_result_t
handle_exit (mos6502_t * cpu)
{
	INFO_PRINT("Received Paravirtual Exit Request. Goodbye.");
	exit(EXIT_SUCCESS);
}

static inline mos6502_step_result_t
handle_dump (mos6502_t * cpu)
{
    uint16_t addr = 0;
    uint32_t count = 12288;

    printf("EXIT STATE:\n");
    printf("PC: 0x%04x\n", cpu->pc);
    printf("SP: 0x%04x\n", cpu->sp);
    printf(" A: 0x%02x\n", cpu->a);
    printf(" X: 0x%02x\n", cpu->x);
    printf(" Y: 0x%02x\n", cpu->y);
    printf(" P: 0x%02x\n", cpu->p.val);

	for (uint32_t n = (uint32_t)((count + 3) / 4); n; n--, addr += 4) {
		printf("  $%04x: %02x %02x %02x %02x\n",
			   (uint16_t)addr,
			   membus_read(cpu->bus, (uint16_t)addr),
			   membus_read(cpu->bus, (uint16_t)addr + 1),
			   membus_read(cpu->bus, (uint16_t)addr + 2),
			   membus_read(cpu->bus, (uint16_t)addr + 3));
	}

    exit(EXIT_SUCCESS);
}

static inline mos6502_step_result_t
handle_open (mos6502_t * cpu)
{
	char path[1024];
	int oflag = 0;

	uint16_t mode  = pop_parm(cpu, cpu->y - 4);
	uint16_t flags = pop_parm(cpu, 2);
	uint16_t name  = pop_parm(cpu, 2);

	uint16_t i = 0;
	do {
		path[i] = (char)membus_read(cpu->bus, name++);
	} while (path[i++]);

	switch (flags & 0x03) {
		case 0x01:
			oflag |= O_RDONLY;
			break;
		case 0x02:
			oflag |= O_WRONLY;
			break;
		case 0x03:
			oflag |= O_RDWR;
			break;
	}

	if (flags & 0x10) {
		oflag |= O_CREAT;
	}

	if (flags & 0x20) {
		oflag |= O_TRUNC;
	}

	if (flags & 0x40) {
		oflag |= O_APPEND;
	}

	if (flags & 0x80) {
		oflag |= O_EXCL;
	}

	// keep gcc from complaining about unused mode
	(void)mode;

	ssize_t ret = open(path, oflag);
	if (ret < 0) {
		ERROR_PRINT("Tried to open file %s with flags %x: %s\n", path, oflag,strerror(errno));
	}
	set_ax(cpu, (uint16_t)ret);

	return MOS6502_STEP_RESULT_SUCCESS;
}

static inline mos6502_step_result_t
handle_close (mos6502_t * cpu)
{
	int fd = get_ax(cpu);
	uint16_t ret = (uint16_t)close(fd);
	set_ax(cpu, ret);
	return MOS6502_STEP_RESULT_SUCCESS;
}

static inline mos6502_step_result_t
handle_read (mos6502_t * cpu)
{
	uint16_t count = get_ax(cpu);
	uint16_t buf = pop_parm(cpu, 2);
	int fd = pop_parm(cpu, 2);

	uint8_t * data = malloc(count);
	ssize_t ret = read(fd, data, count);

	if (ret != -1) {
		uint16_t i = 0;
		while (i < ret) {
			membus_write(cpu->bus, buf++, data[i++]);
		}
	}

	free(data);
	set_ax(cpu, (uint16_t)ret);

	return MOS6502_STEP_RESULT_SUCCESS;
}

static inline mos6502_step_result_t
handle_write (mos6502_t * cpu)
{
	uint16_t i = 0, count = get_ax(cpu), buf = pop_parm(cpu, 2);
	int fd = pop_parm(cpu, 2);


	uint8_t * data = malloc(count);
	while (i < count) {
		data[i++] = membus_read(cpu->bus, buf++);
	}
	uint16_t ret = (uint16_t)write(fd, data, count);

	free(data);
	set_ax(cpu, ret);

	return MOS6502_STEP_RESULT_SUCCESS;
}

mos6502_step_result_t
handle_vmcall (mos6502_t * cpu, uint8_t call_num)
{
	switch (call_num) {
	case VMCALL_ARGS:
		return handle_args(cpu);
	case VMCALL_EXIT:
		return handle_exit(cpu);
	case VMCALL_OPEN:
		return handle_open(cpu);
	case VMCALL_CLOSE:
		return handle_close(cpu);
	case VMCALL_READ:
		return handle_read(cpu);
	case VMCALL_WRITE:
		return handle_write(cpu);
	case VMCALL_BREAK:
		return MOS6502_STEP_RESULT_VMBREAK;
    case VMCALL_DUMP:
        return handle_dump(cpu);
	default:
		return MOS6502_STEP_RESULT_UNHANDLED_VMCALL;
	}
}
