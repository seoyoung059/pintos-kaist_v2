#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	/* %rax는 시스템 콜 번호
	 * 인자는 %rdi, %rsi, %rdx, %r10, %r8, %r9 순서
	 * 시스템 콜의 return값도 rax의 값 수정하여 전달
	 * 
	 * 시스템 콜 번호를 통해 시스템 콜 요청
	 * parameter list 내의 포인터들의 유효성 검사
	 * 	- user area의 가상주소를 가리켜야 함
	 *  - 유효한 주소를 가리키지 않으면 page fault
	 * 유저 스택의 arguments들을 커널로 복사
	 * rax 레지스터에 시스템 콜의 return값 저장
	 */
	printf ("system call!\n");

	int system_call_num = f->R.rax;


	switch(system_call_num){
		case SYS_HALT:                   /* Halt the operating system. */
			{
				syscall_halt();
				break;
			}
		case SYS_EXIT:                   /* Terminate this process. */
			{// void exit (int status)
				int status = f->R.rdi;
				syscall_exit(status);		
				break;	
			}
		case SYS_FORK:                   /* Clone current process. */
			{// pid_t fork (const char *thread_name)
				char* thread_name = f->R.rdi;
				return syscall_fork(thread_name);
				break;
			}
		case SYS_EXEC:                   /* Switch current process. */
			{// int exec (const char *file)
				char* file =f->R.rdi;
				return syscall_exec(file);
				break;
			}
		case SYS_WAIT:                   /* Wait for a child process to die. */
			{// int wait (pid_t pid)
				int pid = f->R.rdi;
				return syscall_wait(pid);
				break;
			}
		case SYS_CREATE:                 /* Create a file. */
			{// bool create (const char *file, unsigned initial_size)
				char* file = f->R.rdi;
				unsigned initial_size = f->R.rsi;
				return syscall_create(file, initial_size);
				break;
			}
		case SYS_REMOVE:                 /* Delete a file. */
			{//bool remove (const char *file)
				char* file = f->R.rdi;
				return syscall_remove(file);
				break;
			}
		case SYS_OPEN:                   /* Open a file. */
			{// int open (const char *file)
				char* file = f->R.rdi;
				return syscall_open(file);
				break;
			}
		case SYS_FILESIZE:               /* Obtain a file's size. */
			{// int filesize (int fd)
				int fd = f->R.rdi;
				return syscall_filesize(fd);
				break;
			}
		case SYS_READ:                   /* Read from a file. */
			{//int read (int fd, void *buffer, unsigned size)
				int fd = f->R.rdi;
				unsigned size = f->R.rdx;
				return syscall_read(fd, f->R.rsi, size);
				break;
			}
		case SYS_WRITE:                  /* Write to a file. */
			{// int write (int fd, const void *buffer, unsigned size)
				int fd = f->R.rdi;
				unsigned size = f->R.rdx;
				return syscall_write(fd, f->R.rsi, size);
				break;
			}
		case SYS_SEEK:                   /* Change position in a file. */
			{//void seek (int fd, unsigned position)
				int fd = f->R.rdi;
				unsigned position = f->R.rsi;
				return syscall_seek(fd, position);
				break;
			}
		case SYS_TELL:                   /* Report current position in a file. */
			{//unsigned tell (int fd)
				int fd = f->R.rdi;
				return syscall_tell(fd);
				break;
			}
		case SYS_CLOSE: 
			{// void close (int fd)
				int fd = f->R.rdi;
				syscall_close(fd);
				break;
			}
	}
	thread_exit ();
}

void syscall_halt(void)
{
	shutdown_power_off();
}

void syscall_exit (int status)
{
	struct thread *cur = thread_current();
	/* Save exit status at process descriptor */
	printf("%s: exit(%d)\n", cur -> name, status);
	thread_exit();
}

int syscall_exec(const char *cmd_line)
{
	return process_exec();
}

int syscall_wait(int pid)
{
	
}