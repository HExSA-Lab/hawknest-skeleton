#pragma once

#include <reset_manager.h>
#include <mos6502/mos6502.h>

int pageforty_setup (reset_manager_t * nonnull rm, mos6502_t * nonnull cpu, const char * nonnull cscheme_path);
