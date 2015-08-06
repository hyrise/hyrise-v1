// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "prefetching.h"

#ifndef NO_PREFETCHING
#include <asm/msr.h>
#include <sched.h>
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>



#ifndef _CPU_COUNT
#define _CPU_COUNT 8
#endif

uint64_t read_prefetch(int cpu) {

#ifndef NO_PREFETCHING
  char msr_file_name[64];
  int fd;

  snprintf(msr_file_name, sizeof(msr_file_name), "/dev/cpu/%d/msr", cpu);
  fd = open(msr_file_name, O_RDONLY);

  if (fd < 0) {
    std::cout << "Something went totally wrong when reading the MSR" << std::endl;
    exit(2);
  }

  uint64_t data;

  if (pread(fd, &data, sizeof(data), MSR_IA32_MISC_ENABLE) != sizeof(data)) {
    std::cout << "Could not read from MSR for CPU" << std::endl;
    exit(2);
  }

  close(fd);

  return data;
#else
  return 0;
#endif
}

void write_prefetch(int cpu, uint64_t data) {
#ifndef NO_PREFETCHING
  char msr_file_name[64];
  int fd;

  snprintf(msr_file_name, sizeof(msr_file_name), "/dev/cpu/%d/msr", cpu);
  fd = open(msr_file_name, O_WRONLY);

  if (fd < 0) {
    std::cout << "Something went totally wrong when writing the MSR" << std::endl;
    std::cout << msr_file_name << std::endl;
    std::cout << fd << std::endl;
    exit(2);
  }

  if (pwrite(fd, &data, sizeof(data), MSR_IA32_MISC_ENABLE) != sizeof(data)) {
    std::cout << "Could not write to MSR for CPU" << std::endl;
    exit(2);
  }

  close(fd);
#endif
}

int lock_process_to_cpu(int cpu) {
#ifndef NO_PREFETCHING
  cpu_set_t m;
  CPU_ZERO(&m);
  CPU_SET(cpu, &m);

  if (sched_setaffinity(getpid(), sizeof(m), &m) < 0) {
    std::cout << "Sched Affin Error (lock)" << std::endl;
    exit(3);
  }

  return cpu;
#else
  return 0;
#endif
}

int lock_process_to_random_cpu() {
#ifndef NO_PREFETCHING
  cpu_set_t m;
  CPU_ZERO(&m);
  int cpu = random() % _CPU_COUNT;
  CPU_SET(cpu, &m);

  if (sched_setaffinity(getpid(), sizeof(m), &m) < 0) {
    std::cout << "Sched Affin Error (lock)" << std::endl;
    exit(3);
  }

  return cpu;
#else
  return 0;
#endif
}

void unlock_process() {
#ifndef NO_PREFETCHING
  cpu_set_t m;
  CPU_ZERO(&m);

  for (int i = 0; i < _CPU_COUNT; ++i) {
    CPU_SET(i, &m);
  }

  if (sched_setaffinity(getpid(), sizeof(m), &m) < 0) {
    std::cout << "Sched Affin Error (unlock)" << std::endl;
    exit(3);
  }

#endif
}


void prefetch_all_off(int cpu) {
  uint64_t value = 0xe4628c0289;
  write_prefetch(cpu, value);
}

void prefetch_l2_off(int cpu) {
  uint64_t value = 0x44628c0289;
  write_prefetch(cpu, value);
}

void prefetch_data_logic(int cpu) {
  uint64_t value = 0x44628c0089;
  write_prefetch(cpu, value);
}

void prefetch_stream(int cpu) {
  uint64_t value = 0x4462840089;
  write_prefetch(cpu, value);
}
