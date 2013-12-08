// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#pragma once

#include <stdint.h>

uint64_t read_prefetch(int cpu);

void write_prefetch(int cpu, uint64_t data);

int lock_process_to_cpu(int cpu);

int lock_process_to_random_cpu();

void unlock_process();

void prefetch_all_off(int cpu);

void prefetch_l2_off(int cpu);

void prefetch_data_logic(int cpu);

void prefetch_stream(int cpu);

