#ifndef frtos_h
#define frtos_h
//this function needs to be called within a section where no interrupt happen, so to make possible no race conditions
void OS_init(void);
VOID OS_onStartup(void);

void OS_sched(void);

//TCB , Thread Control Block
typedef struct {
	void *sp; // stack pointer, that points to the stack of that thread
}OSThread;

typedef void (*OSthreadHandler) (void) ; // function pointer
// needed to create the stack frame for each thread we will call
void OSThread_start ( OSThread *me ,
		      OSThreadHandler threadHandler,
		      void *stkSto , uint32_t stkSize);

#endif;
