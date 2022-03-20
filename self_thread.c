#include "self_thread.h"
#include <string.h>
thread_tptr create_thread(char *job_name, int th_id, char *priority, int cancel_mode)
{
    //change priority into integer
    int p;
    switch(priority[0])
    {
    case 'H':
        p = 2;
        break;
    case 'M':
        p = 1;
        break;
    default:
        p = 0;
        break;
    }
    thread_t *new_th = malloc(sizeof(thread_t));
    new_th->th_name = malloc(1000*sizeof(char));
    strcpy(new_th->th_name,job_name);
    //new_th->th_name = job_name;
    new_th->th_id = th_id;//th_id = th_num(original), th_num = th_num+1
    new_th->b_priority = new_th->th_priority = p;
    new_th->th_cancelmode = cancel_mode;
    new_th->th_cancel_status = 0;
    new_th->th_wait = -1;
    new_th->th_qtime = 0;
    new_th->th_wtime = 0;
    new_th->th_waittime = 0;
    new_th->th_already_wait = 0;
    new_th->th_next = NULL;
    return new_th;
}

//queue: H->H->...M->M->M...->L, previous H is earlier
void enq(thread_tptr *new_th, thread_tptr *head)
{
    thread_tptr temp = (*head);
    thread_tptr temp_ex = NULL;
    if(temp!=NULL)
    {
        while(temp != NULL)
        {
            if(temp->th_priority >= (*new_th)->th_priority)
            {
                temp_ex = temp;
                temp = temp->th_next;
            }
            else
            {
                (*new_th)->th_next = temp;
                if(temp != (*head))
                    temp_ex->th_next = (*new_th);
                else
                    (*head) = (*new_th);
                break;
            }
        }
        if(temp == NULL)
            temp_ex ->th_next = (*new_th);
    }
    else//empty queue
    {
        (*head) = (*new_th);
        (*new_th)->th_next = NULL;
    }
    return;
}

thread_tptr deq(thread_tptr *head)
{
    if((*head) == NULL)
        return NULL;
    else
    {
        thread_tptr leave = (*head);
        (*head) = (*head)->th_next;
        leave->th_next = NULL;
        return leave;
    }
}

void priority_change(thread_tptr *target, int time_past, int tq)
{
    //change priority
    char c[3] = {'L', 'M', 'H'};
    if(time_past < tq)
    {
        if((*target)->th_priority != 2)
        {
            (*target)->th_priority++;
            printf("The priority of thread %s is changed from %c to %c\n",
                   (*target)->th_name, c[(*target)->th_priority-1], c[(*target)->th_priority]);
        }
    }
    else
    {
        if((*target)->th_priority != 0)
        {
            (*target)->th_priority--;
            printf("The priority of thread %s is changed from %c to %c\n",
                   (*target)->th_name, c[(*target)->th_priority+1], c[(*target)->th_priority]);
        }
    }
    return;
}
