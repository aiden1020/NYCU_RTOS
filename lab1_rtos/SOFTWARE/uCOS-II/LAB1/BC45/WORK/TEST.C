/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        3       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/
typedef struct {
    int exeTime;    
    int period;     
} PeriodData;

char MessageBuffer[buffer_size][50];
int buffer_counter = 0;
int TimeStart = 0;
int msgIndex = 0;

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
PeriodData    TaskData[N_TASKS] = { {1, 3},{3, 6},{4, 9} };


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/
        void  PrintAllMessage();
        void  Task(void *data);                       /* Function prototypes of tasks                  */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
        void  PrintAllMessage();

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */


    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[100];
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    TaskStartCreateTasks();                                /* Create all the application tasks         */
	OSTimeSet(0);
	TimeStart = 1;
    for (;;) {
        PrintAllMessage();


        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}



/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static void TaskStartCreateTasks(void)
{
    int i;
    for (i = 0; i < N_TASKS; i++) {
        OSTaskCreate(Task, (void *)&TaskData[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/



void Task(void *data) {
    PeriodData *tmpdata = (PeriodData*) data;
    int C = tmpdata->exeTime;  
    int P = tmpdata->period;   
    int start, end, toDelay;

    OS_ENTER_CRITICAL();
    OSTCBCur->compTime=C;             
    OSTCBCur->period=P;               
    OS_EXIT_CRITICAL();
    start = OSTimeGet();          
    printf("task%d start at time:%d\n", OSTCBCur->OSTCBPrio, start);
    while (1) {
        while (OSTCBCur->compTime > 0) {
            // do nothing
        }
        OS_ENTER_CRITICAL();
        end = OSTimeGet(); 
        if (end > start+OSTCBCur->period){
            printf("time_tick %d task%d miss deadline!\n", start+OSTCBCur->period, OSTCBCur->OSTCBPrio);
        }
        toDelay = P - (end - start);
        start += P;
        OSTCBCur->compTime = C; 
        OS_EXIT_CRITICAL();
        OSTimeDly(toDelay);
    }
}

void PrintAllMessage(){
    while (msgIndex < buffer_counter) {
        printf("%s", MessageBuffer[msgIndex]);
        msgIndex++;
    }
}
