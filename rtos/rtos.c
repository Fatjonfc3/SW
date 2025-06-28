#include <stdint.h>
#include "frtos.h"
#include "qassert.h" //qpc library
Q_DEFINE_THIS_FILE

OSThread * volatile OS_curr; // bcs they will be used in interrupt we use volatile after the asterisk , so the pointer is volatile, like the value the pointer points not the address where the pointer is stored
OSThread * volatile OS_next;

OSThread *OS_thread [32 + 1] ; //to store all the existing threads available, it would be better to have a linked list but its ok
uint8_t OS_threadNum ; //how many threads have been started
uint8_t OS_currIdx;
//needs to happen at the beginning
void OS_init(void) {
*(uint32_t volatile *) 0xE00ED20 | = (0xFFU  << 16 ) ; tu put higher priority to the sys tick bcs at this register at 0xE00ED20 , the first 3MSB, 
define the priority for the systick so by oring with this all the rest are the same except the 4 msb , even if we put 1111 the arm will put 111x so dont worry 
}


void OS_run(void) {
	//callback to configure and start interrupts
	OS_onStartup();
	__disable_irq(
	OS_sched();
	__enable_irq(); --till now the 

	Q_ASSERT(0); // technically the code should never get here , bcs os_Sched will ched the first task and pass control to it , since the schedule in fact is done in pendsv interrupt then the interrupt will preeempt the OS_run 
	//Q_ERROR();

}

void OS_sched(void) {
//OS_next =;
if (OS_curr = NULL){

OS_currIdx = 0;
}else { 
++OS_currIdx;
if ( OS_currIdx == OS_threadnum) {
	OS_currIdx = 0U;
} 
OS_next = OS_thread [OS_currIdx];
}
if (OS_next != OS_current) {
	*(uint32_t volatile *)0xE000ED04 = (1u << 28) ; //the register to trigger the pendsv exception
}
}



void OSThread_start ( OSThread *me ,
		      OSThreadHandler threadHandler,
		      void *stkSto , uint32_t stkSize)
{

Q_ASSERT (OS_threadNum < MAX_THREAD) ; //IF false it calls Q_onAssert callback , it is defined bcs its used by startup code
//don't use just an if else bcs the caller may not care about the error we pass


	OS_thread[OS_threadNum] = me;
	threadNum++; 


uint32_t *sp = (uint32_t*)(((uint32t_t)stkSto + stkSize)/8)*8) ;
//floor rounding, we just want to align with 8 bytes , so the last 3 bits should be 0
// bcs then the number will be 2**3 + 2**4 + 2**5 and so on that can be factored by 8
 //because in ARM Processors , stack goes from high memory to low one , we need to start at the endk, stack needs to be aligned at 8 byte boundary , so if we divide by 8 an integer number then we have rounded so by multiplying by 8 we are aligned
//prefill the remaining stack with arbitrary data
uint32_t *stk_limit = (uint32_t) ((((uint32t_t)stkSto -1) /8 ) + 1 ) * 8)
//ceil rounding, so round up bcs we don't want to get an address that is 8 byte aligned but from not our space, -1 is needed just if the address is already aligned to not go up
//other option also jus tstkSto + 7  & ~0x7
//we round uo the bottom of the stack since the last address is the end thats why -1 
*(--sp) = (1U << 24); //xPSR , decrement first and then store, holds apsr (zero , saturated , overflow etc), ipsr (if in handler mode) , epsr
*(--sp) = (uint32_t)threadHandler ; //program handelr,bcs unction is already a pointer to function , the thread handler
*(--sp) = 0x0000000EU; // LR
*(--sp) = 0x0000000CU; // R12
*(--sp) = 0x00000002U; // R2
*(--sp) = 0x00000001U; // R1
*(--sp) = 0x00000000U; // R0 
//Till now , these are the callee saving registers , but we store also
// the caller saved bcs the functions that may interrupt our code , and when we may take the control may not be the end of the function , so the interrupting code may not restore the caller saved or may not store them , so we need to store them 
*(--sp) = 0x0000000BU; // R11
*(--sp) = 0x0000000AU; // R10
*(--sp) = 0x00000009U; // R9
*(--sp) = 0x00000008U; // R8
*(--sp) = 0x00000007U; // R7
*(--sp) = 0x00000006U; // R6
*(--sp) = 0x00000005U; // R5
*(--sp) = 0x00000004U; // R4
//-- save the top of the stack to the thread structure we created
me->sp = sp;

for (sp = sp - 1U ; sp >= stk_limit ; --sp) {
	*sp = 0xDEADBEEFU ;
}
}
__asm
void PendSV_Handler(void) {
//Only after SysTick handler the pendsv needs to run
//so we place the right priority so sys tick has higher priority then pendsv handler
//cutting though the confusuion with arm cortex m interrupt priorities
void *sp; //wrote this just when the assembly is generated to use the reg where it stores that value
__disable_irq();
if (OS_curr != (OSThread *)0){
	//push regs r4 to r11 on the stack, mm to the currThread stack
	OS_curr-> sp = sp;
}
sp = OS_next -> sp;
OS_curr = OS_next
//pop regs r4 to r11 from the new stack bcs we changed the sp, and the sp will point at the end of that stack where we have saved all the regs before the context switching
__enable_irq();
IMPORT OS_curr //extern variable
IMPORT OS_next
CPSID  I ; // disables interrupts

//comparison of os curr , for the os curr address we use =variable_name, assembly offers this option,pra laoded me address
LDR r1,=OS_curr 
LDR r1 , [r1 , #0x00]
CBZ r1,PendSV_restore //so ont branch if the os_curr is not equal to 0
//push regs r4-r11 the ones that haven't be stored from the interrupt
PUSH {r4-r11}
//store now the current sp to the sp data structure
LDR r1,=OS_curr
LDR r1 , [r1,#0x00] --will store the address where the sp in data structure, bcs we have pointers inside a pointer
//then store current sp reg to that address
STR sp , [r1, #0x00]

PendSV_restore
/* SP = os_NEXT->sp
ldr r1, =OS_next
LDR r1 , [r1 , #0x00]
LDR r2 , [r1 , #0x00]
//OS_curr = OS_next
ldr r1 , =OS_next
LDR r1 , [r1 , #0x00]
ldr r2 , =OS_curr
STR r1 , [r2,#0x00] //bcs os_curr a pointer , so first we get the address of the pointer and then the value of that pointer that is the address of the data structure in realitty
//now pop the r4-r11 regs that the bx exec_return doesn't pop immediately
pop {r4-r11}

CPSIE I

//return of exc function gets the sp and pops the values and then goes to the correct sp , that's why it does the context switch
BX lr






} //just get an exception


void SysTick_Handler(void){
__disable_irq()'
OS_sched();
__enable_irq();
}
