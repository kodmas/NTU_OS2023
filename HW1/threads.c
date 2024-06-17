#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0


static struct thread* current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
//static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;

    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    return t;
    //printf("create thread success\n");
}
void thread_add_runqueue(struct thread *t){
    if(current_thread == NULL){
        //t->sig_handler[0] = current_thread->sig_handler[0];
        //t->sig_handler[0] = current_thread->sig_handler[1];
        // TODO
        current_thread = t;
        current_thread->next = current_thread;
        current_thread->previous = current_thread;

    }
    else{
        t->sig_handler[0] = current_thread->sig_handler[0];
        t->sig_handler[1] = current_thread->sig_handler[1];
        // TODO
        current_thread->previous->next = t;
        t->previous = current_thread->previous;
        t->next = current_thread;
        current_thread->previous = t;

    }
}
void thread_yield(void){
    // TODO
    if(current_thread->signo != -1){
        if(setjmp(current_thread->handler_env) == NULL){
            //printf("check\n");
            schedule();
           //sleep(2);
            dispatch();
        }
    }
    else{
        //printf("return here\n");
        if(setjmp(current_thread->env) == NULL){
            schedule();
            dispatch();
            //printf("return here\n");
        }
    }
    
    
}
void dispatch(void){
    //printf("dispatch\n");
    //printf("dispatch signo = %d\n",current_thread->signo);
    //part 2
    if(current_thread->signo != -1){
        if(current_thread->sig_handler[current_thread->signo] != NULL_FUNC){
            if(current_thread->handler_buf_set == 1){
                //printf("signo = %d\n",current_thread->signo);
                //printf("11\n");
                //sleep(5);
                longjmp(current_thread->handler_env,1);
            }
            else{
                if(setjmp(current_thread->handler_env) == 1){
                    //printf("22\n");
                    current_thread->sig_handler[current_thread->signo](current_thread->signo);
                    //current_thread->signo = -1;
                    //longjmp(current_thread->handler_env,1);
                    current_thread->signo = -1;
                    //current_thread->handler_env->sp = (unsigned long) current_thread->stack_p;
                    //current_thread->env->sp = (unsigned long) current_thread->stack_p;
                    //schedule();
                    //longjmp(current_thread->env,1);
                    dispatch();
                }
                else{
                    //printf("33\n");
                    current_thread->handler_env->sp = (unsigned long) current_thread->stack_p-50*8;
                    current_thread->handler_buf_set = 1;
                    longjmp(current_thread->handler_env,1);
                }
            }
        }
        else{
            //printf("ext\n");
            thread_exit();
        }
    }
    else{
        // TODO
        if(current_thread->buf_set == 1){ // current_thread->env != NULL
            //printf("1\n");
            //current_thread->handler_env->sp = (unsigned long) current_thread->stack_p;
            //printf("signo = %d\n",current_thread->signo);
            //printf("%d\n",current_thread->env->sp);
            longjmp(current_thread->env,1);
            //printf("a\n");
        }
        else{ // current_thread->env == NULL
            if(setjmp(current_thread->env) == 1){ //set jmpbuf success, exec func
                //printf("2\n");
                current_thread->fp(current_thread->arg);
                
            }
            else{
                //printf("3\n");
                current_thread->env->sp =(unsigned long) current_thread->stack_p;
                current_thread->buf_set = 1; //manual set jmpbuf finished
                longjmp(current_thread->env,1);
            }
        }
        thread_exit();
    }
}
void schedule(void){
    // TODO
    //printf("schedule\n");
    current_thread = current_thread->next;

}
void thread_exit(void){
    if(current_thread->next != current_thread){
        // TODO
        struct thread *curr = current_thread;
        current_thread->previous->next = current_thread->next;
        current_thread->next->previous = current_thread->previous;
        schedule();
        free(curr->stack);
        free(curr);
        dispatch();
    }
    else{
        // TODO
        // Hint: No more thread to execute
        free(current_thread->stack);
        free(current_thread);
        longjmp(env_st,1); //return to main function
    }
}
void thread_start_threading(void){
    // TODO
    if(setjmp(env_st) == 0){
        schedule();
        dispatch();
    }
    else return;
}
// part 2
void thread_register_handler(int signo, void (*handler)(int)){
    // TODO
    current_thread->sig_handler[signo] = handler;
    //printf("set handler success\n");
    sleep(3);
}
void thread_kill(struct thread *t, int signo){
    // TODO
    t->signo = signo;
    /*
    if(t->sig_handler[signo] != NULL_FUNC){
        //printf("%p\n",t->sig_handler[signo]);
        //printf("%p\n",NULL_FUNC);
        //printf("thread not killed\n");
        //change some variable 
        //t->sig_handler[signo](signo);
        //dispatch();
    }
    else{
        //printf("thread killed\n");
        //thread_exit();
    }*/
}