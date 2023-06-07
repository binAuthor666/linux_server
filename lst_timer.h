#ifndef LST_TIMER
#define LST_TIMER

#include<time.h>
#include<stdio.h>
#define BUFFER_SIZE 64
class util_timer;

/*用户数据结构*/
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

/*定时器类*/
class util_timer
{
public:
    util_timer():prev(NULL),next(NULL) {}
public:
    time_t expire;/*任务的超时时间*/
    void (*cb_func)(client_data*);/*任务回调函数*/
    client_data* user_data;
    util_timer* prev;
    util_timer* next;
};

/*定时器链表，升序、双向链表、带有头节点和尾节点*/
class sort_timer_list
{
public:
    sort_timer_list():head(NULL),tail(NULL){}
    /*链表被销毁时，删除其中所有的定时器*/
    ~sort_timer_list()
    {
        util_timer* tmp=head;
        while(tmp)
        {
            head=tmp->next;
            delete tmp;
            tmp=head;
        }
    }
    /*将目标定时器timer添加到链表中*/
    void add_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        if(!head)
        {
            head=tail=timer;
            return;
        }

        if(timer->expire < head->expire)
        {
            timer->next=head;
            head->prev=timer;
            head=timer;
            return;
        }
        add_timer(timer,head);
    }
    /*当某个定时任务发生变化时，调整对应的定时器在链表中的位置*/
    void adjust_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }
        util_timer* tmp=timer->next;

        if(!tmp||(timer->expire<tmp->expire))
        {
            return;
        }

        if(timer==head)
        {
            head=head->next;
            head->prev=NULL;
            timer->next=NULL;
            add_timer(timer,head);
        }
        else
        {
            timer->prev->next=timer->next;
            timer->next->prev=timer->prev;
            add_timer(timer,timer->next);
        }
    }
    /*将目标定时器timer从链表中删除*/
    void del_timer(util_timer* timer)
    {
        if(!timer)
        {
            return;
        }

        if((timer==head)&&(timer==tail))
        {
            delete timer;
            head=NULL;
            tail=NULL;
            return;
        }

        if(timer==head)
        {
            head=head->next;
            head->prev=NULL;
            delete timer;
            return;
        }

        if(timer==tail)
        {
            tail=tail->prev;
            tail->next=NULL;
            delete timer;
            return;
        }

        timer->prev->next=timer->next;
        timer->next->prev=timer->prev;
        delete timer;
    }
    /*SIGALRM信号每次被触发就在其信号处理函数中执行一次tick函数，以处理链表上到期的任务*/
    void tick()
    {
        if(!head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur=time(NULL);/*获得系统当前的时间*/
        util_timer* tmp=head;

        while(tmp)
        {
            if(cur < tmp->expire)
            {
                break;
            }

            tmp->cb_func(tmp->user_data);

            head=tmp->next;
            if(head)
            {
                head->prev=NULL;
            }
            delete tmp;
            tmp=head;
        }
    }
private:
/*该函数表示将目标定时器timer添加到节点lst_head之后的部分链表中*/
void add_timer(util_timer* timer,util_timer* lst_head)
{
    util_timer* prev=lst_head;
    util_timer* tmp=prev->next;

    while(tmp)
    {
        if(timer->expire < tmp->expire)
        {
            prev->next=timer;
            timer->next=tmp;
            tmp->prev==timer;
            timer->prev=prev;
            break;
        }
        prev=tmp;
        tmp=tmp->next;
    }

    if(!tmp)
    {
        prev->next=timer;
        timer->prev=prev;
        timer->next=NULL;
        tail=timer;
    }
}

private:
    util_timer* head;
    util_timer* tail;
};

#endif