/*
 * TaskScheduler.c
 *
 * Created: 6/17/2024 5:04:37 PM
 * Author : MSI
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define MAX_TASKS (5)		//Maximum number of tasks

// task states
#define RUNNABLE (0x00)		// Ready
#define RUNNING  (0x01)
#define STOPPED  (0x02)
#define ERROR    (0x03)

#define redLED PD2			//Assign Port D bit4 (PD2) to redED
#define greenLED PD3		//Assign Port D bit4 (PD3) to greenLED
#define yellowLED PD4		//Assign Port D bit4 (PD4) to yellowLED
#define blueLED PD5			//Assign Port D bit4 (PD5) to blueLED


// pointer to a void function with no arguments:type task_t
typedef void (*task_t)(void);

// Basic task control block (TCB)
typedef struct __tcb_t
{
	uint8_t id;			// Task ID
	task_t task;		// Pointer to the task
	uint16_t delay;		// Delay before execution
	uint16_t period;	// Interval between subsequent runs
	uint8_t status;		// Status of task
} tcb_t;

// Declare scheduler functions
void initScheduler(void);
void addTask(uint8_t, task_t, uint16_t);
void deleteTask(uint8_t);
uint8_t getTaskStatus(uint8_t);
void dispatchTasks(void);

// Declare prototypes of tasks
void Task1(void);
void Task2(void);
void Task3(void);
void Task4(void);

// The task list
tcb_t task_list[MAX_TASKS];		// Define task list
uint16_t count = 0;				// Keeps track of number of timer interrupts

int main(void)
{
	initScheduler();		// Set up the task list
	
	//Add tasks
	addTask(1, Task1, 1);	// Task1 runs every 1 second
	addTask(2, Task2, 2);	// Task2 runs every 2 seconds
	addTask(3, Task3, 3);	// Task3 runs every 3 seconds
	addTask(4, Task4, 6);	// Task4 runs every 6 seconds
	
	sei();	// Enable all interrupts
	
	while(1)
	{
		dispatchTasks();
	}
	return 0;
}



// Initialization Function 
void initScheduler(void)
{
	TCNT1 = 0xC2F7;							// Set for 1 sec
	TCCR1A = 0x00;							// Normal mode
	TCCR1B |= ((1 << CS12) | (1 << CS10));  // Use 1024 prescalar
	TIMSK1 = 0x01;							// Enable Timer1 Overflow interrupt
	DDRD = 0xFF;							// Set PORTD as output
	
	for(uint8_t i=0; i<MAX_TASKS; i++)
	{
		task_list[i].id = 0;
		task_list[i].task = (task_t)0x00;	//set an null pointer of task_t type
		task_list[i].delay = 0;
		task_list[i].period = 0;
		task_list[i].status = STOPPED;
	}
}

// Adds a new task to the task list.
// Scans through the list and places the new task data where
// it finds free space.
void addTask(uint8_t id, task_t task, uint16_t period)
{
	uint8_t idx = 0, done = 0x00;
	while( idx < MAX_TASKS )
	{
		if( task_list[idx].status == STOPPED )
		{
			task_list[idx].id = id;
			task_list[idx].task = task;
			task_list[idx].delay = period;
			task_list[idx].period = period;
			task_list[idx].status = RUNNABLE;
			done = 0x01;
		}
		if( done ) break;
		idx++;
	}
}

// Remove task from task list
// Note STOPPED is equivalent to removing a task
void deleteTask(uint8_t id)
{
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		if( task_list[i].id == id )
		{
			task_list[i].status = STOPPED;
			break;
		}
	}
}

// Gets the task status
// Returns ERROR if id is invalid
uint8_t getTaskStatus(uint8_t id)
{
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		if( task_list[i].id == id )
		return task_list[i].status;
	}
	return ERROR;
}

// Dispatches tasks when they are ready to run
void dispatchTasks(void)
{
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		// Check for a valid task ready to run
		if( !task_list[i].delay && task_list[i].status == RUNNABLE )
		{
			task_list[i].status = RUNNING;				// Task is now running
			(*task_list[i].task)();						// Call the task
			task_list[i].delay = task_list[i].period;	// Reset the delay
			task_list[i].status = RUNNABLE;				// Task is runnable again
		}
	}
}
/* ----------------------------------------------------------------------- */

/* ------------------------------- ISR ----------------------------------- */
// Generate "ticks"
// Each tick 1s apart
ISR(TIMER1_OVF_vect)
{
	TCNT1 = 0xC2F7;					// Reset Timer1 to 0xC2F7 (1 sec)
	TIFR1 = 0x01;					// Clear overflow flag
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		if( task_list[i].status == RUNNABLE )
		task_list[i].delay--;
	}
}
/* --------------------------------------------------------------------- */

/* --------------------------- Task definitions ------------------------ */
void Task1(void)
{
	static uint8_t status = 1;
	if(status == 0x01)
	{
		PORTD |= 0b00000100;	//Set portD bit2 HIGH
	}
	
	else
	{
		PORTD &= ~0b00000100;	//Set portD bit2 LOW
	}
	status = !status;
}

void Task2(void)
{
	static uint8_t status = 1;
	if(status == 0x01 )
	{
		PORTD |= 0b00001000;	//Set portD bit3 HIGH
	}
	
	else
	{
		PORTD &= ~0b00001000;	//Set portD bit3 LOW
	}
	status = !status;
}

void Task3(void)
{
	static uint8_t status = 1;
	if(status == 0x01 )
	{
		PORTD |= 0b00010000;	//Set portD bit4 HIGH
	}
	
	else
	{
		PORTD &= ~0b00010000;	//Set portD bit4 LOW
	}
	status = !status;
}

void Task4(void)
{
	static uint8_t status = 1;
	if(status == 0x01 )
	{
		PORTD |= 0b00100000;	//Set portD bit5 HIGH
	}
	
	else
	{
		PORTD &= ~0b00100000;	//Set portD bit5 LOW
	}
	status = !status;
}

