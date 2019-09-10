# Hawknest Internals
This document gives an overview of the internal workings of Hawknest, and will
hopefully be useful to those wishing to modify or improve them. It's not
comprehensive of all the functions or data types; documentation for those (if
it's been written) will be in the corresponding headers (or source files, for
file-private entities).

## Reference Counting

- `emu/include/rc.h`
- `emu/src/rc.c`

Hawknest uses some pseudo-object-oriented patterns to enable limited
polymorphism, and the most pronounced instance of this is the
reference-counting system. Every heap-allocated "object" stores a reference
count and a pointer to a deinitialization function, so it can be shared
opaquely and destroyed by its last user.

Objects allocated through the reference-counting system have their reference
count and deinitializer stored _before_ the object instance in memory, so there
are no extra steps for dereferencing a pointer to a reference-counted object.
This also means that the _type_ of a reference-counted pointer is no different
from a regular one, so the code in Hawknest uses a commenting convention to
document the nature of each object _referee_ held by a _referrer_:

- A field marked as `/*strong*/` is a strong reference held to the referee;
  this means that the referee's reference count should be incremented
  (`rc_retain()`) when the reference is taken by the referrer, and decremented
  (`rc_release()`) when the referrer no longer holds it, or is itself
  deinitialized.

- A field marked as `/*weak*/` is a weak reference held to the referee.
  Semantics are similar to `/*strong*/` references, except that
  `rc_weak_check()` is used before each dereference, and the retain/release
  functions are replaced by `rc_weak_retain()` and `rc_weak_release()`. Weak
  references are used by referrers which only care about referees so long as
  they're being used elsewhere, such as the reset manager.

- A field marked as `/*owned*/` is allocated and managed without using the
  reference-counting system (in Hawknest usually because it's an SDL object).

- A field marked as `/*unowned*/` is allocated using the reference-counting
  system, but assumed to be valid for the lifetime of the reference.  This is
  often used to break reference cycles, where the lifetime of the referrer is
  guaranteed to be bounded by the lifetime of the referee.

For the most part, reference-counted objects are only retained when references
are stored in objects that are themselves reference-counted. Newly-created
objects always have a reference count of 1, and there's rarely a need to retain
a reference within the call stack (though this is done occasionally). There's
no hard convention like in automatically-reference-counted languages; we just do
what works!

## Reset Management and Sequencing

- `emu/include/reset_manager.h`
- `emu/src/reset_manager.c`

Since Hawknest models real hardware, it follows that it should also model
hardware reset sequences. Both before the start of emulated execution, and
whenever the "reset button" is hit, every component should revert to its
initial state. Since Hawknest's object graph has no clean way to apply
ownership, a separate _reset manager_ was devised to make sure that the reset
routines of every individual component are invoked once, and only once, at each
global system reset. For this reason, most device constructors take a reference
to a `reset_manager_t`, with which they register their reset routines.

For the majority of devices, resetting simply involves flushing registers and
memories to default values. A primary exception is the 6502, which has a reset
sequence it executes over multiple cycles, after receiving the initial reset
signal. The reset manager (in its current iteration) has no support for
guaranteed reset order, leading to problematic race conditions when stateful
mappers are involved. Currently, the CPU is just reset separately from (and
after) all other devices attatched to the reset manager, through its own
manually-invoked reset routine. This is not an ideal solution, and certainly
not scalable to the implementation of other devices with non-instant reset
semantics.

It's also worth noting that, in real hardware (and with the NES), there's often
a difference between the "cold" and "warm" reset states. Hawknest makes no
distinction, and instead tries to treat every reset as a cold restart.

## Timekeeping

- `emu/include/timekeeper.h`
- `emu/src/timekeeper.c`

With the introduction of a component like the PPU, Hawknest becomes a
relatively complex concurrent system, and keeping a notion of "real time" along
with the emulated cycle timings becomes less trivial. Really, Hawknest has to
deal with two kinds of time, with separate but interwoven concerns:

- _Virtual_ time is just the number of virtual clock cycles that have elapsed
  at any point during emulation. We want to make sure that the various system
  components are emulated with as much cycle-accuracy as possible, at least to
  the extent that they can interact and observe each other's state.

- _Real_ time is the wall-clock time at any point during execution. The NES had
  no notion of wall-clock time, instead relying on its system clock to control
  game speed, and so we need to emulate the "speed" of the NES hardware to make
  games run at the right speed.

There are a few ways to potentially deal with this, but Hawknest takes the
following approach:

- The virtual CPU (6502) _drives everything_. Virtual time in the system only
  progresses because the CPU is being run and executing instructions.

- Virtual devices that conceptually run concurrent with the CPU (the only such
  device currently implemented is the PPU) are emulated lazily, and so play
  catchup to the current virtual time point when their state becomes relevant
  to simulation.

- Whenever a device performs an IO action that requires interaction with the
  real world (e.g. sampling button presses on a controller), the main thread is
  put to sleep until virtual time is aligned with real time. This presumes that
  the emulation will be on average running faster than real time, which is
  generally the case when building it with compiler optimizations. If it's not,
  no sleeping is done.

In order to make this happen, Hawknest implements a timekeeper, a small object
that keeps track of the relationship between real and virtual time, and
provides an easy-to-call function (`timekeeper_sync()`) that puts the calling
thread to sleep as described above, handling the calculation of how long to
actually sleep for.

### Timers
The scheme described above works very nicely for almost all system operation,
but some IO actions (such as displaying frames of video to the screen) should
happen regardless of whether or not the CPU initiates an interaction that
causes a lazy update of a concurrent device. Worse, some external devices (the
PPU included) generate asynchronous hardware interrupts, and there's no way to
recover if the emulator misses an interrupt. To deal with these problems,
Hawknest adds another responsibility to the timekeeper: managing timers.

Conceptually, the timekeeper maintains a list of simple count-down timers,
running in virtual time (i.e. counts are measured in clock cycles). All ticks
of the virtual clock are done through the timekeeper
(`timekeeper_advance_clk()`), and so the timekeeper knows immediately when a
timer has reached the end of its countdown. When this happens, the timekeeper
executes a corresponding handler function (registered as part of timer
creation), which implements whatever hardware-asynchronous action needs to have
happened at or by that virtual time.
