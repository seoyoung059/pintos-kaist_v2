#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
struct lock filesys_lock;
struct lock load_lock;
#endif /* userprog/syscall.h */
