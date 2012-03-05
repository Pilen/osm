/*
 * Process startup.
 *
 * Copyright (C) 2003-2005 Juha Aatrokoski, Timo Lilja,
 *       Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: process.c,v 1.11 2007/03/07 18:12:00 ttakanen Exp $
 *
 */

#include "proc/process.h"
#include "proc/elf.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "kernel/sleepq.h"
#include "fs/vfs.h"
#include "drivers/yams.h"
#include "vm/vm.h"
#include "vm/pagepool.h"

#include "lib/libc.h"

/** @name Process startup
 *
 * This module contains a function to start a userland process.
 */

/** Spinlock which must be held when manipulating the process table */
spinlock_t process_table_slock;

/** The table containing all processes in the system, whether active or not */
process_table_t process_table[CONFIG_MAX_PROCESSES];

void process_clear(process_id_t pid);

/**
 * Starts one userland process. The thread calling this function will
 * be used to run the process and will therefore never return from
 * this function. This function asserts that no errors occur in
 * process startup (the executable file exists and is a valid ecoff
 * file, enough memory is available, file operations succeed...).
 * Therefore this function is not suitable to allow startup of
 * arbitrary processes.
 *
 * @executable The name of the executable to be run in the userland
 * process
 */
void process_start(const char *executable)
{
    thread_table_t *my_entry;
    pagetable_t *pagetable;
    uint32_t phys_page;
    context_t user_context;
    uint32_t stack_bottom;
    elf_info_t elf;
    openfile_t file;

    int i;

    interrupt_status_t intr_status;

    my_entry = thread_get_current_thread_entry();

    /* If the pagetable of this thread is not NULL, we are trying to
       run a userland process for a second time in the same thread.
       This is not possible. */
    KERNEL_ASSERT(my_entry->pagetable == NULL);

    pagetable = vm_create_pagetable(thread_get_current_thread());
    KERNEL_ASSERT(pagetable != NULL);

    intr_status = _interrupt_disable();
    my_entry->pagetable = pagetable;
    _interrupt_set_state(intr_status);

    file = vfs_open((char *)executable);
    /* Make sure the file existed and was a valid ELF file */
    KERNEL_ASSERT(file >= 0);
    KERNEL_ASSERT(elf_parse_header(&elf, file));

    /* Trivial and naive sanity check for entry point: */
    KERNEL_ASSERT(elf.entry_point >= PAGE_SIZE);

    /* Calculate the number of pages needed by the whole process
       (including userland stack). Since we don't have proper tlb
       handling code, all these pages must fit into TLB. */
    KERNEL_ASSERT(elf.ro_pages + elf.rw_pages + CONFIG_USERLAND_STACK_SIZE
		  <= _tlb_get_maxindex() + 1);

    /* Allocate and map stack */
    for(i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - i*PAGE_SIZE, 1);
    }

    /* Allocate and map pages for the segments. We assume that
       segments begin at page boundary. (The linker script in tests
       directory creates this kind of segments) */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.ro_vaddr + i*PAGE_SIZE, 1);
    }

    for(i = 0; i < (int)elf.rw_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.rw_vaddr + i*PAGE_SIZE, 1);
    }

    /* Put the mapped pages into TLB. Here we again assume that the
       pages fit into the TLB. After writing proper TLB exception
       handling this call should be skipped. */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);
    
    /* Now we may use the virtual addresses of the segments. */

    /* Zero the pages. */
    memoryset((void *)elf.ro_vaddr, 0, elf.ro_pages*PAGE_SIZE);
    memoryset((void *)elf.rw_vaddr, 0, elf.rw_pages*PAGE_SIZE);

    stack_bottom = (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - 
        (CONFIG_USERLAND_STACK_SIZE-1)*PAGE_SIZE;
    memoryset((void *)stack_bottom, 0, CONFIG_USERLAND_STACK_SIZE*PAGE_SIZE);

    /* Copy segments */

    if (elf.ro_size > 0) {
	/* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.ro_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.ro_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.ro_vaddr, elf.ro_size)
		      == (int)elf.ro_size);
    }

    if (elf.rw_size > 0) {
	/* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.rw_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.rw_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.rw_vaddr, elf.rw_size)
		      == (int)elf.rw_size);
    }


    /* Set the dirty bit to zero (read-only) on read-only pages. */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        vm_set_dirty(my_entry->pagetable, elf.ro_vaddr + i*PAGE_SIZE, 0);
    }

    /* Insert page mappings again to TLB to take read-only bits into use */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);

    /* Initialize the user context. (Status register is handled by
       thread_goto_userland) */
    memoryset(&user_context, 0, sizeof(user_context));
    user_context.cpu_regs[MIPS_REGISTER_SP] = USERLAND_STACK_TOP;
    user_context.pc = elf.entry_point;

    thread_goto_userland(&user_context);

    KERNEL_PANIC("thread_goto_userland failed.");
}


/** Calls process start after a thread has been created.
 * Takes its pid as parameter
 */
void process_launch(uint32_t pid) {
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  thread_get_current_thread_entry()->process_id = (process_id_t) pid;

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  process_start(process_table[pid].executable);

}

process_id_t process_spawn(const char *executable) {
  static process_id_t next_pid = 0;
  process_id_t i, pid = -1;
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  //Find an available spot in the process table.
  for (i=0; i<CONFIG_MAX_PROCESSES; i++) {
    process_id_t p = (i + next_pid) % CONFIG_MAX_PROCESSES;

    if (process_table[p].state == PROCESS_FREE) {
      pid = p;
      break;
    }
  }

  //Is the process table full?
  if (pid < 0) {
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return pid;
  }

  //Let next_pid point to the next position in the array
  next_pid = (pid+1) % CONFIG_MAX_PROCESSES;

  //Initialize table
  stringcopy(process_table[pid].executable, executable, CONFIG_MAX_FILENAME_SIZE);
  process_table[pid].pid = pid;
  process_table[pid].state = PROCESS_RUNNING;
  process_table[pid].result = 0;
  process_table[pid].program_counter = 0;
  process_table[pid].parent = process_get_current_process();
  process_table[pid].child = -1;
  process_table[pid].next_sibling = -1;

  //You shall be my child
  i = process_table[process_get_current_process()].child;
  if (i < 0) {
    process_table[process_get_current_process()].child = pid;
  }
  else {
    while (process_table[i].next_sibling >= 0) {
      i = process_table[i].next_sibling;
    }
    process_table[i].next_sibling = pid;
  }

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  thread_run(thread_create(process_launch, pid));


  return pid;
}

/** 
 *
 * Asserts thread isn't already running a process.
 */
int process_run(const char *executable) {
  static process_id_t next_pid = 0;
  process_id_t i, pid = -1;
  interrupt_status_t intr_status;

  KERNEL_ASSERT(thread_get_current_thread_entry()->process_id < 0);

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  //Find an available spot in the process table.
  for (i=0; i<CONFIG_MAX_PROCESSES; i++) {
    process_id_t p = (i + next_pid) % CONFIG_MAX_PROCESSES;

    if (process_table[p].state == PROCESS_FREE) {
      pid = p;
      break;
    }
  }

  //Is the process table full?
  if (pid < 0) {
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return pid;
  }

  //Let next_pid point to the next position in the array
  next_pid = (pid+1) % CONFIG_MAX_PROCESSES;

  //Initialize table
  stringcopy(process_table[pid].executable, executable, CONFIG_MAX_FILENAME_SIZE);
  process_table[pid].pid = pid;
  process_table[pid].state = PROCESS_RUNNING;
  process_table[pid].result = 0;
  process_table[pid].program_counter = 0;
  process_table[pid].parent = -1;
  process_table[pid].child = -1;
  process_table[pid].next_sibling = -1;

  //let the thread know of us
  thread_get_current_thread_entry()->process_id = (process_id_t) pid;

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  process_start(executable);

  return 0;
}
process_id_t process_get_current_process(void) {
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  process_id_t current_process_id = thread_get_current_thread_entry()->process_id;
  _interrupt_set_state(intr_status);

  return current_process_id;
}

void process_finish(int retval) {
  interrupt_status_t intr_status;
  process_id_t pid;
  process_id_t child;

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  pid = process_get_current_process();

  // Itterate over all the children
  child = process_table[pid].child;
  while (child >= 0) {
    // If child is zombie, clear it
    if (process_table[child].state == PROCESS_ZOMBIE)
      process_clear(child);
    // Else its an orphan
    else
      process_table[child].parent = -1;

    child = process_table[child].next_sibling;
  }


  //If process has a parent, wait to be joined
  if (process_table[pid].parent >= 0) {
    process_table[pid].result = retval;
    process_table[pid].state = PROCESS_ZOMBIE;
  } else {
    process_clear(pid);
  }

  kwrite("Process finishing - wake parents\n");

  //Mommy might be sleeping, lets wake her up.
  sleepq_wake_all(&(process_table[pid].pid));

  vm_destroy_pagetable(thread_get_current_thread_entry()->pagetable);
  thread_get_current_thread_entry()->pagetable = NULL;

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  thread_finish();
}

int process_join(process_id_t pid) {
  interrupt_status_t intr_status;
  int result;

  //pid must be legal;
  KERNEL_ASSERT(pid < CONFIG_MAX_PROCESSES);
  KERNEL_ASSERT(pid >= 0);

  //Processes can only join children;
  KERNEL_ASSERT(process_table[pid].parent == process_get_current_process());

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  while (process_table[pid].state != PROCESS_ZOMBIE) {
    sleepq_add(&(process_table[pid].pid));
    spinlock_release(&process_table_slock);
    thread_switch();
    spinlock_acquire(&process_table_slock);
  }
  result = process_table[pid].result;
  process_clear(pid);

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);
  return result;
}

void process_init(void) {
  int i;
  spinlock_reset(&process_table_slock);
  
  /* Init all entries */
  for (i=0; i<CONFIG_MAX_PROCESSES; i++) {
    process_clear(i);
  }
}

/** Clear a table element from the process_table.
 *
 * Note that obtaining process_table_slock and disabling interrupts
 * is assumed to be done by calling function if needed.
 */
void process_clear(process_id_t pid) {

  KERNEL_ASSERT(pid < CONFIG_MAX_PROCESSES);
  
  stringcopy(process_table[pid].executable, "\0", CONFIG_MAX_FILENAME_SIZE);
  //process_table[pid].executable = "\0";
  process_table[pid].pid = -1;
  process_table[pid].state = PROCESS_FREE;
  process_table[pid].result = 0;
  process_table[pid].program_counter = 0;
  process_table[pid].parent = -1;
  process_table[pid].child = -1;
  process_table[pid].next_sibling = -1;
}

/** @} */
