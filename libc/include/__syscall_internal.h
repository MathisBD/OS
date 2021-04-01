
// system calls takes their arguments in registers, 
// in the following order :
// eax, ebx, ecx, edx, edi, esi
// the first argument is always the system call number
// the return value is put in eax

// syscall interrupt number
#define SC_INTR 0x80

#define SC_NEW_THREAD       1
#define SC_NEW_PROCESS      2
#define SC_EXIT             3

#define SC_ARG_0    %eax
#define SC_ARG_1    %ebx
#define SC_ARG_2    %ecx
#define SC_ARG_3    %edx
#define SC_ARG_4    %edi
#define SC_ARG_5    %esi
