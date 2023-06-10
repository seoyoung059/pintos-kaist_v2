#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"

#include "filesys/file.h"
#include "filesys/filesys.h"
#include "devices/input.h"
#include "lib/string.h"
#include "threads/palloc.h"
// #include "user/syscall.h"
// #include ""

// #include "u"
#include "userprog/process.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

void halt(void);

int exit (int status);
int fork (const char *thread_name, struct intr_frame *f);
int exec(const char *cmd_line);
int wait(int pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int set_fd(struct file* f);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

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

	lock_init(&filesys_lock);
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
	// printf ("system call!\n");

	int system_call_num = f->R.rax;


	switch(system_call_num){
		case SYS_HALT:                   /* Halt the operating system. */
			{
				// printf("halt\n\n");
				halt();
				break;
			}
		case SYS_EXIT:                   /* Terminate this process. */
			{// void exit (int status)
				// printf("exit\n\n");
				f->R.rax = exit(f->R.rdi);		
				break;	
			}
		case SYS_FORK:                   /* Clone current process. */
			{// pid_t fork (const char *thread_name)
				// printf("fork\n\n");
				f->R.rax = fork(f->R.rdi,f);
				break;
			}
		case SYS_EXEC:                   /* Switch current process. */
			{// int exec (const char *file)
				// printf("exec\n\n");
				// f->R.rax = 
				f->R.rax = exec(f->R.rdi);
				break;
			}
		case SYS_WAIT:                   /* Wait for a child process to die. */
			{// int wait (pid_t pid)
				// printf("wait\n\n");
				f->R.rax = wait(f->R.rdi);
				break;
			}
		case SYS_CREATE:                 /* Create a file. */
			{// bool create (const char *file, unsigned initial_size)
				// printf("create\n\n");
				f->R.rax = create(f->R.rdi, f->R.rsi);
				break;
			}
		case SYS_REMOVE:                 /* Delete a file. */
			{//bool remove (const char *file)
				// printf("remove\n\n");
				f->R.rax = remove(f->R.rdi);
				break;
			}
		case SYS_OPEN:                   /* Open a file. */
			{// int open (const char *file)
				// printf("open\n\n");
				f->R.rax = open(f->R.rdi);
				break;
			}
		case SYS_FILESIZE:               /* Obtain a file's size. */
			{// int filesize (int fd)
				// printf("filesize\n\n");
				f->R.rax = filesize(f->R.rdi);
				break;
			}
		case SYS_READ:                   /* Read from a file. */
			{//int read (int fd, void *buffer, unsigned size)
				// printf("read\n\n");
				f->R.rax =  read(f->R.rdi, f->R.rsi, f->R.rdx);
				break;
			}
		case SYS_WRITE:                  /* Write to a file. */
			{// int write (int fd, const void *buffer, unsigned size)
				// printf("write\n\n");
				f->R.rax = write(f->R.rdi, f->R.rsi, f->R.rdx);
				break;
			}
		case SYS_SEEK:                   /* Change position in a file. */
			{//void seek (int fd, unsigned position)
				// printf("seek\n\n");
				seek(f->R.rdi, f->R.rsi);
				break;
			}
		case SYS_TELL:                   /* Report current position in a file. */
			{//unsigned tell (int fd)
				// printf("tell\n\n");
				f->R.rax = tell(f->R.rdi);
				break;
			}
		case SYS_CLOSE: 
			{// void close (int fd)
				// printf("close\n\n");
				close(f->R.rdi);
				break;
			}
		default: thread_exit ();
	}
}




// void addr_check(void* ptr){
// 	/* check if the user-provided ptr is in user pool
// 	 * 	1. 페이지 테이블 체크해서 매핑되었는지 확인
// 	 * 			- 유효성 검사 이후 lock이나 할당 가능
// 	 * 	2. PHYS_BASE 아래의 user pointer check
// 	 * 			- is_user_vaddr
// 	 * 			- is_kern
// 	 */
// 	if(!is_kernel_vaddr(ptr)) exit(-1);
// }



void halt(void)
{
	power_off();
}

int exit (int status)
{
	/* 현재 돌아가고 있는 유저 프로그램을 종료하고, 커널에 status를 return
	 * 만약 부모 프로세스가 wait중이라면, 여기서 반환하는 status를 반환할 것.
	 * 관습적으로 status=0은 성공이고 그 외엔 에러를 나타냄
	 */
	struct thread *cur = thread_current();
	printf("%s: exit(%d)\n", cur -> name, status);
	thread_current()->exit_status=status;
	sema_up(&thread_current()->wait_sema);
	thread_exit();

	return status;
}

int fork (const char *thread_name, struct intr_frame *f)
{
	/* TODO: thread_name이라는 이름으로 현재 프로세스의 복사본인 새 프로세스를 만듦.
	 * callee-saved 레지스터인 %RBX, %RSP, %RBP, and %R12 - %R15 외의 레지스터의
	 * 값을 복사할 필요 없음. 자식 프로세스의 pid를 return해야하고 그 외에는
	 * 유효한 pid를 return해서는 안됨. 자식 프로세스에서는, return value가 0이여야 함.
	 * 자식 프로세스는 fd와 가상 메모리 공간을 포함한 복사된 자원들을 가져야 한다.
	 * 부모 프로세스는 자식 프로세스가 성공적으로 복사되었는지 알기 전에는 return
	 * 해서는 안된다. 즉, 자식 프로세스가 자원을 복사하지 못하면, 부모의 fork()호출은
	 * TID_ERROR나게 되어있을 것이다.
	 * threads/mmu.c의 pml4_for_each()를 사용하여 전체 유저 메모리 공간을 복사하게 되어있지만,
	 * 전달된 pte_for_each_func의 빈 부분을 채워넣어야 한다.
	 */
	return process_fork(thread_name,f);
	
}

int exec(const char *cmd_line)
{
	/* FIXME: 현재 프로세스를 cmd_line의 실행가능 프로그램과 인자로 바꾼다.
	 * 성공하면 return하지 않고, 그 외에는 exit state -1로 프로세스를
	 * 끝내고, 그 어떤 이유로도 load나 실행이 안될 것이다.
	 * 함수는 exec을 실행한 쓰레드의 이름을 바꾸지 않는다. fd는 exec call을 지나도
	 * 남는다. 
	 */
	char *fn_copy;

	/* Make a copy of FILE_NAME.
	 * Otherwise there's a race between the caller and load(). */
	fn_copy = palloc_get_page (0);
	if (fn_copy == NULL)
		return TID_ERROR;
	strlcpy (fn_copy, cmd_line, PGSIZE);
	if(process_exec(fn_copy)==-1) exit(-1);
}

int wait(int pid)
{// TODO:
	/* pid를 갖는 child process가 끝날때까지 기다리고, 그 exit status를 반환
	 * pid 프로세스가 아직 살아있으면, 종료할때까지 기다리고, pid 프로세스가 exit으로 
	 * 전달한 status를 return. pid가 exit()을 호출하지 않고, 커널에 의해 종료되면,
	 * wait는 -1을 반환.
	 * parent는 child 프로세스 pid가 이미 종료된 시점에 wait을 호출할 수 있으며, 커널은
	 * 해당 child 프로세스의 exit staus를 반환할 수 있게/커널에 의해 종료된 것을 확인할
	 * 수 있게 해 주어야 함.
	 * *wait이 fail하여 -1을 return하는 경우
	 * 		- pid 프로세스가 wait을 호출하는 프로세스의 직접적인 자식이 아닐 때 
	 * 		- wait를 호출하는 프로세스가 이미 wait를 호출했을 때.
	 * 				한 프로세스는 그 자식 프로세스들에 대해 한 시점에 하나의 wait만 할 수 있음
	//  */
	// // pid를 갖는 child process 찾기
	// struct thread* child = get_current_child(pid);
	// // child process가 끝날때까지 기다리고 (sema up은 child가 종료할 때)
	// sema_down(&child->wait_sema);
	// // exit status 확인
	// return child->exit_status;
	// // return 81;
	return process_wait(pid);
}

bool create (const char *file, unsigned initial_size)
{
	if (file==NULL) exit(-1);
	return filesys_create(file, initial_size);
}

bool remove (const char *file)
{
	return filesys_remove(file);
}

int set_fd(struct file* f){
	int i=2;
	struct file** fdt = thread_current()->fdt;
	lock_acquire(&filesys_lock);
	for(i=2; i < 64 && fdt[i] != NULL; i++) continue;
	if(i < 64)
	{	thread_current()->fdt[i]=f;
		lock_release(&filesys_lock);
		return i;
	}
	else{
		lock_release(&filesys_lock);
		return -1;
	}
}


int open (const char *file){
	/* file 경로의 파일을 열고, fd를 return
	 * 열 수 없으면 -1 return
	 * fd 0,1은 stdin/stdout을 위해 reserved되어있으므로 사용 X
	 * fd는 child process에 inherited
	 * file이 여러번 open되면 새 fd 반환
	 * 한 file에 대한 서로 다른 파일 디스크립터는 각각 독립적으로 닫히고,
	 * file position을 공유하지 않음
	 */
	if (file==NULL) return -1;
	struct file *new_file = filesys_open(file);
	if (new_file==NULL) return -1;
	
	int fd = set_fd(new_file);
	return fd;
}


int filesize (int fd)
{
	struct file* f = thread_current()->fdt[fd];
	return file_length(f);
}

int read (int fd, void *buffer, unsigned size)
{
	//실패하면 -1 return -> 언제 실패하지?
	// fd가 유효하지 않을 때? (연결된 파일이 없을때?)
	if(fd == STDIN_FILENO){
		int i;
		for (i=0;i<size;i++){
			input_getc();
		}
		return size;
	}
	else if (fd==STDOUT_FILENO){
		return -1;
	}
	else {
		lock_acquire(&filesys_lock);
		struct file* f = (thread_current()->fdt)[fd];
		if (f==NULL) return -1;
		off_t ans = file_read(f,buffer,size);
		lock_release(&filesys_lock);
		return ans;
	}
}

int write (int fd, const void *buffer, unsigned size)
{	
	// printf("\n\n@@@@write\n\n");
	if (fd==STDIN_FILENO) {
		return -1;
	}
	else if (fd==STDOUT_FILENO){
		putbuf(buffer,size);
		return size;
	}
	else {
		lock_acquire(&filesys_lock);
		struct file* f = thread_current()->fdt[fd];
		off_t val = file_write(f, buffer, size);
		lock_release(&filesys_lock);
		return val;
	}
}

void seek (int fd, unsigned position)
{
	struct file* f = thread_current()->fdt[fd];
	file_seek(f, position);
}

unsigned tell (int fd)
{
	struct file* f = thread_current()->fdt[fd];
	return file_tell(f);
}

void close (int fd)
{
	struct file* f = thread_current()->fdt[fd];
	if(f==NULL) exit(-1);
	file_close(f);
	thread_current()->fdt[fd]=NULL;
}