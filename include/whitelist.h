#ifndef OJ_WHITELIST_H
#define OJ_WHITELIST_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h>

typedef struct {
    uint8_t list[1024 / sizeof(uint8_t)];
} Whitelist;

static inline void whitelist_add(Whitelist *wl, int32_t syscall_number) {
    div_t dm = div(syscall_number, 8);
    wl->list[dm.quot] |= (0x80 >> dm.rem);
}

static inline bool whitelist_contains(Whitelist *wl, int32_t syscall_number) {
    div_t dm = div(syscall_number, 8);
    return (wl->list[dm.quot] & (0x80 >> dm.rem)) != 0;
}

static inline void whitelist_erase(Whitelist *wl, int32_t syscall_number) {
    div_t dm = div(syscall_number, 8);
    wl->list[dm.quot] &= ~(0x80 >> dm.rem);
}

static inline void whitelist_init(Whitelist *wl) {
    whitelist_add(wl, SYS_read);
    whitelist_add(wl, SYS_write);
    whitelist_add(wl, SYS_fstat);
    whitelist_add(wl, SYS_mmap);
    whitelist_add(wl, SYS_mprotect);
    whitelist_add(wl, SYS_munmap);
    whitelist_add(wl, SYS_brk);
    whitelist_add(wl, SYS_access);
    whitelist_add(wl, SYS_exit_group);
    // whitelist_add(wl, SYS_execve);  // !!!  -> remember to remove it when
    // first hit
    whitelist_add(wl, SYS_arch_prctl);  // !!!
    whitelist_add(wl, SYS_uname);       // !!!
    whitelist_add(wl, SYS_readlink);    // !!!
}

static inline void whitelist_reset(Whitelist *wl) {
    whitelist_add(wl, SYS_execve);
}

#endif