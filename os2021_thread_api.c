#include "os2021_thread_api.h"
#include "cJSON.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

struct itimerval Signaltimer;
long time_past = 0;
ucontext_t dispatch_context;
ucontext_t timer_context;
ucontext_t finish_context;
thread_pointer now_execute = NULL;
thread_pointer ready_queue_head = NULL;
thread_pointer wait_queue_head = NULL;
thread_pointer terminate_queue_head = NULL;
int thread_num = 1;
char priority_three[3] = {'L', 'M', 'H'};
int time_quantum[3] = {300, 200, 100};
thread_pointer create_thread(char *job_name, int thread_tid, char *priority, int cancel_mode)
{
    int p ;
    if(strcmp(priority,"L") == 0)
    {
        p = 0;
    }
    else if (strcmp(priority,"M") == 0)
    {
        p = 1;
    }
    else if(strcmp(priority,"H") == 0)
    {
        p = 2;
    }
    thread_pointer new_thread = malloc(sizeof(thread));
    new_thread->th_name = malloc(1000*sizeof(char));
    strcpy(new_thread->th_name,job_name);
    new_thread->thread_tid = thread_tid;//thread_tid = th_num(original), th_num = th_num+1
    new_thread->b_priority = new_thread->c_priority = p;
    new_thread->thread_cancel_mode = cancel_mode;
    new_thread->thread_cancel_status = 0;
    new_thread->thread_wait = -1;
    new_thread->thread_qtime = 0;
    new_thread->thread_wtime = 0;
    new_thread->thread_waittime = 0;
    new_thread->thread_already_wait = 0;
    new_thread->thread_next = NULL;
    return new_thread;
}

void enq(thread_pointer *new_thread, thread_pointer *head)
{
    if((*head)==NULL)
    {
        (*head) = (*new_thread);
        (*new_thread)->thread_next = NULL;
    }
    else
    {
        thread_pointer temp = (*head);
        thread_pointer temp_ex = NULL;
        while(temp != NULL)
        {
            if(temp->c_priority >= (*new_thread)->c_priority)
            {
                temp_ex = temp;
                temp = temp->thread_next;
            }
            else
            {
                (*new_thread)->thread_next = temp;
                if(temp != (*head))
                    temp_ex->thread_next = (*new_thread);
                else
                    (*head) = (*new_thread);
                break;
            }
        }
        if(temp == NULL)
        {
            temp_ex ->thread_next = (*new_thread);
            (*new_thread)->thread_next = NULL;
        }
    }

    return;
}
thread_pointer deq(thread_pointer *head)
{
    if((*head) == NULL)
    {
        return NULL;
    }
    thread_pointer leave = (*head);
    (*head) = (*head)->thread_next;
    leave->thread_next = NULL;
    return leave;

}

void priority_change(thread_pointer *target, int time_past, int time_quantum)
{
    int p = (*target)->c_priority;
    if(time_past < time_quantum)
    {
        if(p < 2)
        {
            p++;
            printf("The priority of thread %s is changed from %c to %c\n",(*target)->th_name, priority_three[p-1], priority_three[p]);
        }
    }
    else
    {
        if(p > 0)
        {
            p--;
            printf("The priority of thread %s is changed from %c to %c\n",(*target)->th_name, priority_three[p+1], priority_three[p]);
        }
    }
    (*target)->c_priority = p;
    return;
}
int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode)
{
    //printf("Name:%s\tFunction:%s\tPriority:%s\tCancel Mode:%d\n",job_name,p_function,priority,cancel_mode);
    if(strcmp(p_function, "Function1") != 0 && strcmp(p_function, "Function2") != 0 && strcmp(p_function, "Function3") != 0 && strcmp(p_function, "Function4") != 0 && strcmp(p_function, "Function5") != 0 && strcmp(p_function, "ResourceReclaim") != 0)
    {
        return -1;
    }
    thread_pointer new_thread = create_thread(job_name, thread_num, priority, cancel_mode);
    if(strcmp(p_function, "Function1") == 0)
    {
        CreateContext(&(new_thread->thread_ctx), &finish_context, &Function1);
        thread_num+=1;
    }
    else if(strcmp(p_function, "Function2") == 0)
    {
        CreateContext(&(new_thread->thread_ctx), &finish_context, &Function2);
        thread_num+=1;

    }
    else if(strcmp(p_function, "Function3") == 0)
    {
        CreateContext(&(new_thread->thread_ctx), &finish_context, &Function3);
        thread_num+=1;
    }
    else if(strcmp(p_function, "Function4") == 0)
    {
        CreateContext(&(new_thread->thread_ctx), &finish_context, &Function4);
        thread_num+=1;
    }
    else if(strcmp(p_function, "Function5") == 0)
    {
        CreateContext(&(new_thread->thread_ctx), &finish_context, &Function5);
        thread_num+=1;

    }
    else if(strcmp(p_function, "ResourceReclaim") == 0)
    {
        CreateContext(&(new_thread->thread_ctx), NULL, &ResourceReclaim);
        thread_num+=1;

    }
    enq(&new_thread, &ready_queue_head);
    return new_thread->thread_tid;
}
void OS2021_ThreadCancel(char *job_name)
{
    thread_pointer target = NULL;
    thread_pointer temp_th = ready_queue_head;
    thread_pointer ex_th = NULL;
    if(strcmp("reclaimer", job_name)==0)
    {
        return;//reclaimer can't enter terminate state
    }
    while (temp_th!=NULL)
    {
        if(strcmp(temp_th->th_name, job_name)==0)
        {
            target = temp_th;
            if(target->thread_cancel_mode == 0)
            {
                if(target == ready_queue_head)
                {
                    ready_queue_head = target->thread_next;
                }
                else
                {
                    ex_th->thread_next = target->thread_next;
                }
                enq(&target, &terminate_queue_head);
            }
            else
            {
                target->thread_cancel_status = 1;
                printf("%s wants to cancel thread %s\n", now_execute->th_name, target->th_name);
            }
            return;
        }
        else
        {
            ex_th = temp_th;
            temp_th = temp_th->thread_next;
        }
    }
    if(target == NULL)
    {
        temp_th = wait_queue_head;
        ex_th = NULL;
        while(temp_th != NULL)
        {
            if(strcmp(temp_th->th_name, job_name) == 0)
            {
                target = temp_th;
                if(target->thread_cancel_mode == 0)
                {
                    if(target == wait_queue_head)
                    {
                        wait_queue_head = target->thread_next;
                    }
                    else
                    {
                        ex_th->thread_next = target->thread_next;
                    }
                    enq(&target, &terminate_queue_head);
                }
                else
                {
                    target->thread_cancel_status = 1;
                    printf("%s wants to cancel thread %s\n", now_execute->th_name, target->th_name);
                }
                return;
            }
            else
            {
                ex_th = temp_th;
                temp_th = temp_th->thread_next;
            }
        }
    }
    return;
}

void OS2021_ThreadWaitEvent(int event_id)
{
    thread_pointer target = now_execute;
    target->thread_wait = event_id;
    printf("%s wants to waiting for event %d\n", target->th_name, event_id);
    priority_change(&target, time_past, time_quantum[target->c_priority]);//change priority
    enq(&target, &wait_queue_head);//change to wait queue
    swapcontext(&(target->thread_ctx), &dispatch_context);//save current status and reschedule
    return;

}

void OS2021_ThreadSetEvent(int event_id)
{
    thread_pointer return_th = NULL;
    thread_pointer temp_th = wait_queue_head;
    thread_pointer ex_th = NULL;
    //try to find target
    while(temp_th != NULL)
    {
        while(temp_th->thread_wait != event_id)
        {
            ex_th = temp_th;
            temp_th = temp_th->thread_next;
        }
        temp_th->thread_wait = -1;
        if(temp_th == wait_queue_head)
            wait_queue_head = wait_queue_head->thread_next;
        else
        {
            ex_th->thread_next = temp_th->thread_next;
        }
        return_th = temp_th;
        return_th->thread_next = NULL;
        printf("%s changes the status of %s to READY.\n", now_execute->th_name, return_th->th_name);
        enq(&return_th, &ready_queue_head);
        return;
    }
    return;
}

void OS2021_ThreadWaitTime(int msec)
{
    thread_pointer target = now_execute;
    priority_change(&target, time_past, time_quantum[target->c_priority]);
    target->thread_waittime = msec;
    target->thread_next = NULL;
    enq(&target, &wait_queue_head);
    swapcontext(&(target->thread_ctx), &dispatch_context);
    return;
}

void OS2021_DeallocateThreadResource()
{
    thread_pointer target = terminate_queue_head;
    while(target != NULL)
    {
        printf("The memory space by %s has been released.\n", target->th_name);
        terminate_queue_head = terminate_queue_head->thread_next;
        free(target);
        target = terminate_queue_head;
    }
    return;
}

void OS2021_TestCancel()
{
    thread_pointer target = now_execute;
    if(target->thread_cancel_status == 1)
    {
        enq(&target, &terminate_queue_head);
        setcontext(&dispatch_context);
    }
    else
    {
        return;
    }
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context,(void (*)(void))func,0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");

    }
}

void Dispatcher()
{
    now_execute = deq(&ready_queue_head);
    time_past = 0;
    ResetTimer();
    //printf("Threadname:%s\n",now_execute->th_name);
    setcontext(&(now_execute->thread_ctx));
}
void FinishThread()
{
    thread_pointer target = now_execute;
    now_execute = NULL;
    enq(&target, &terminate_queue_head);
    setcontext(&dispatch_context);
}

void ControlZ(int signal)
{
    printf("\n");
    printf("**************************************************************************************************\n");
    printf("*\tTID\tName\t\tState\t\tB_Priority\tC_Priority\tQ_Time\tW_time\t *\n");
    printf("*\t%d\t%-10s\tRUNNING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",now_execute->thread_tid, now_execute->th_name, priority_three[now_execute->b_priority], priority_three[now_execute->c_priority], now_execute->thread_qtime, now_execute->thread_wtime);
    thread_pointer temp_th = ready_queue_head;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%-10s\tREADY\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->thread_tid, temp_th->th_name, priority_three[temp_th->b_priority], priority_three[temp_th->c_priority], temp_th->thread_qtime, temp_th->thread_wtime);
        temp_th = temp_th->thread_next;
    }
    temp_th = wait_queue_head;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%-10s\tWAITING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->thread_tid, temp_th->th_name, priority_three[temp_th->b_priority], priority_three[temp_th->c_priority], temp_th->thread_qtime, temp_th->thread_wtime);
        temp_th = temp_th->thread_next;
    }
    printf("**************************************************************************************************\n");
    return;
}
void TimerHandler()
{
    int p = now_execute->c_priority;
    time_past += 10;
    thread_pointer temp_th = ready_queue_head;
    thread_pointer ex_th = NULL;
    while(temp_th != NULL)
    {
        temp_th->thread_qtime += 10;
        temp_th = temp_th->thread_next;
    }
    temp_th = wait_queue_head;
    while(temp_th != NULL)
    {
        temp_th->thread_wtime += 10;
        if(temp_th->thread_waittime != 0)
        {
            thread_pointer target = temp_th;
            thread_pointer target_ex = ex_th;
            target->thread_already_wait ++;
            if(target->thread_already_wait >= target->thread_waittime)
            {
                target->thread_waittime = 0;
                target->thread_already_wait = 0;
                if(target == wait_queue_head)
                    wait_queue_head = wait_queue_head->thread_next;
                else
                    target_ex->thread_next = target->thread_next;
                enq(&target, &ready_queue_head);
            }
        }
        ex_th = temp_th;
        temp_th = temp_th->thread_next;
    }
    if(time_past >= time_quantum[p])
    {

        if(p !=0)
        {
            p--;
            printf("The priority of thread %s is changed from %c to %c\n", now_execute->th_name, priority_three[p+1], priority_three[p]);
            now_execute->c_priority = p;
        }
        enq(&now_execute, &ready_queue_head);
        swapcontext(&(now_execute->thread_ctx), &dispatch_context);
    }
    ResetTimer();
    return;
}

void StartSchedulingSimulation()
{
    /*Set Timer*/
    Signaltimer.it_interval.tv_usec = 100;
    Signaltimer.it_interval.tv_sec = 0;
    ResetTimer();
    signal(SIGALRM, TimerHandler);
    signal(SIGTSTP, ControlZ);
    /*Create Context*/

    CreateContext(&dispatch_context, NULL, &Dispatcher);
    CreateContext(&finish_context, &dispatch_context, &FinishThread);

    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);
    get_json();
    /*printf("FThreadname:%s\t%d \n",ready_queue_head->th_name,ready_queue_head->thread_tid);
    printf("SecondThreadname:%s\t%d\n",ready_queue_head->thread_next->th_name,ready_queue_head->thread_next->thread_tid);
    printf("TThreadname:%s\t%d\n",ready_queue_head->thread_next->thread_next->th_name,ready_queue_head->thread_next->thread_next->thread_tid);
    */
    setcontext(&dispatch_context);
}
void get_json()
{
    //StartSchedulingSimulation();
    int fd = open("./init_threads.json",O_RDWR);
    if(fd < 0)
    {
        perror("open fail\n");
        return -1;
    }
    char buf[2048] = {0};
    int ret = read(fd,buf,sizeof(buf));
    if (ret == -1)
    {
        perror("read error\n");
    }
    close(fd);
    cJSON* root = cJSON_Parse(buf);
    if(root == NULL)
    {
        printf("parse error\n");
    }
    cJSON* value = cJSON_GetObjectItem(root, "Threads");
    int len = cJSON_GetArraySize(value);
    char function_stack[len];
    char priority_stack[len];
    char cancel_mode_stack[len];
    int i = 0;
    cJSON* name = NULL;
    cJSON* enfunction = NULL;
    cJSON* priority = NULL;
    cJSON* canmode = NULL;
    for(i = 0; i<len; i++)
    {
        name = cJSON_GetArrayItem(value,i);
        enfunction = cJSON_GetArrayItem(value,i);
        priority = cJSON_GetArrayItem(value,i);
        canmode = cJSON_GetArrayItem(value,i);

        name = cJSON_GetObjectItem(name,"name");
        enfunction = cJSON_GetObjectItem(enfunction,"entry function");
        priority = cJSON_GetObjectItem(priority,"priority");
        canmode = cJSON_GetObjectItem(canmode,"cancel mode");
        if(name == NULL||enfunction == NULL||priority == NULL||canmode == NULL)
        {
            printf("GETOBJECTITEM FAIL\n");
            return -1;
        }
        char temp[10] = {};
        strcpy(temp,name->valuestring);
        char temp1[20] = {};
        strcpy(temp1,enfunction->valuestring);

        //printf("Fnction:%c\n",temp[1]);
        char temp2[2] = {};
        strcpy(temp2,priority->valuestring);
        //printf("Priority:%c\n",temp2[0]);
        char temp3[2];
        strcpy(temp3,canmode->valuestring);


        function_stack[i] = temp[1];
        priority_stack[i] = temp2[0];
        cancel_mode_stack[i] = temp3[0]-48;
        int q = 0;
        q = OS2021_ThreadCreate(temp,temp1,&priority_stack[i],cancel_mode_stack[i]);
        if(q == -1)
        {
            printf("Incorrect entry function.\n");
        }

    }
}
