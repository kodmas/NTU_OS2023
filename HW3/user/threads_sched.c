#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args)
{   
    struct thread *thread_with_ed = NULL;
    struct thread* curr = NULL;
    int earlist_release_time = 1000000;
    int earlist_deadline_lefttime = NULL; 
    int minID = 10000;
    list_for_each_entry(curr,args.run_queue,thread_list){
       //printf("run current_time = %d, this_t_deadtime = %d, this_t_id = %d\n",args.current_time,curr->current_deadline,curr->ID);
    }
    struct release_queue_entry *cur = NULL;
    struct release_queue_entry *thread_with_ert;
    list_for_each_entry(cur,args.release_queue,thread_list){
        if(cur->release_time < earlist_release_time) {
            earlist_release_time = cur->release_time;
            thread_with_ert = cur;
            //printf("ssd %d\n",cur->thrd->ID);
        }
        //printf("changed %d\n",earlist_release_time);
        //printf("rel current_time = %d, this_t_releasetime = %d\n",args.current_time,cur->release_time);
    }
    list_for_each_entry(curr,args.run_queue,thread_list){
        if(thread_with_ed == NULL || (curr->current_deadline - args.current_time) < earlist_deadline_lefttime ||
        ((curr->current_deadline - args.current_time) == earlist_deadline_lefttime && curr->ID < minID)){
            thread_with_ed = curr;
            earlist_deadline_lefttime = (curr->current_deadline - args.current_time);
            minID = curr->ID;
        }

    }
   // printf("earlist_release_time = %d, earlist_deadline_lefttime = %d\n",earlist_release_time,earlist_deadline_lefttime);
    struct threads_sched_result r;
    if(thread_with_ed != NULL){
        r.scheduled_thread_list_member = &thread_with_ed->thread_list;
        if(args.current_time + thread_with_ed->remaining_time > thread_with_ed->current_deadline){
            r.allocated_time = thread_with_ed->current_deadline - args.current_time;
        }
        else{
            if(earlist_release_time == 1000000) r.allocated_time = thread_with_ed->remaining_time;
            else{
                if((args.current_time + thread_with_ed->remaining_time) < earlist_release_time){
                    r.allocated_time = thread_with_ed->remaining_time;
                }
                else{
                    //printf("ssd %d\n",cur->thrd->ID);
                    if(earlist_release_time + thread_with_ert->thrd->period > thread_with_ed->current_deadline){
                        r.allocated_time = thread_with_ed->remaining_time;
                    }
                    else r.allocated_time = earlist_release_time - args.current_time;
                }
            }
            
        }
        
    }
    else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = (earlist_release_time - args.current_time);
    }
    return r;
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args)
{
    //struct thread *thread_with_ed = NULL;
    struct thread* thread_with_shortest_p = NULL;
    struct thread* curr = NULL;
    int shortest_period = NULL;
    //int earlist_deadline_lefttime = NULL;
    int minID = 10000;
    list_for_each_entry(curr,args.run_queue,thread_list){
       //printf("run current_time = %d, this_t_deadtime = %d, this_t_id = %d\n",args.current_time,curr->current_deadline,curr->ID);
    }
    list_for_each_entry(curr,args.run_queue,thread_list){
        if(thread_with_shortest_p == NULL || curr->period < shortest_period ||
        (curr->period == shortest_period && curr->ID < minID)){
            thread_with_shortest_p = curr;
            shortest_period = curr->period;
            minID = curr->ID;
        }
    }
    int earlist_release_time = 1000000;
    struct release_queue_entry *cur = NULL;
    //struct release_queue_entry *thread_with_ert;
    list_for_each_entry(cur,args.release_queue,thread_list){
        if(cur->release_time < earlist_release_time) {
            earlist_release_time = cur->release_time;
            //thread_with_ert = cur;
            //printf("ssd %d\n",cur->thrd->ID);
        }
        //printf("changed %d\n",earlist_release_time);
        //printf("rel current_time = %d, this_t_releasetime = %d\n",args.current_time,cur->release_time);
    }
    //printf("earlist_release_time = %d\n",earlist_release_time);
    struct threads_sched_result r;
    int missed_dl_thread_id = 10000;
    struct thread* missed_dl_thread = NULL;
    list_for_each_entry(curr,args.run_queue,thread_list){
        if((curr->current_deadline < args.current_time || (curr->current_deadline == args.current_time && curr->remaining_time > 0))
         && curr->ID < missed_dl_thread_id){
            //printf("change id to %d\n",curr->ID);
            missed_dl_thread = curr;
            missed_dl_thread_id = curr->ID;
        }
    }
    if(missed_dl_thread != NULL){
        r.scheduled_thread_list_member = &missed_dl_thread->thread_list;
        r.allocated_time = 0;
        //printf("dddd\n");
        return r;
    }
    if(thread_with_shortest_p != NULL){
        r.scheduled_thread_list_member = &thread_with_shortest_p->thread_list;
        if(args.current_time + thread_with_shortest_p->remaining_time > thread_with_shortest_p->current_deadline){
            r.allocated_time = thread_with_shortest_p->current_deadline - args.current_time;
        }
        else{
             if(args.current_time + thread_with_shortest_p->remaining_time > earlist_release_time){
                r.allocated_time = (earlist_release_time - args.current_time);
            }
            else r.allocated_time = thread_with_shortest_p->remaining_time;
        }
       
    }
    else{
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = (earlist_release_time - args.current_time);
    }
    return r;
}
