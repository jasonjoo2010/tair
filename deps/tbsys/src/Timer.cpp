/*
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version: $Id$
 *
 * Authors:
 *   duolong
 *
 */

#include "Timer.h"
#include "Exception.h"

using namespace std;
namespace tbutil
{
Timer::Timer() :
    Thread(),
    _destroyed(false)
{
    start();
}

void Timer::destroy()
{
    {
        Monitor<Mutex>::Lock sync(_monitor);
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;
        _monitor.notifyAll();
        _tasks.clear();
        _tokens.clear();
    }

    if(!_detachable)
    {
        join();
    }
}

int Timer::schedule(const TimerTaskPtr& task, const TimeObject& delay)
{
    Monitor<Mutex>::Lock sync(_monitor);
    if(_destroyed)
    {
#ifdef _NO_EXCEPTION
        TBSYS_LOG(ERROR,"%s","timer destroyed...");       
        return -1; 
#else
        throw IllegalArgumentException(__FILE__, __LINE__, "timer destroyed");
#endif
    }

    TimeObject time = TimeObject::now(TimeObject::Monotonic) + delay;
    bool inserted = _tasks.insert(make_pair(task, time)).second;
    if(!inserted)
    {
#ifdef _NO_EXCEPTION
        TBSYS_LOG(ERROR,"%s","task is already schedulded...");       
        return -1; 
#else
        throw IllegalArgumentException(__FILE__, __LINE__, "task is already schedulded");
#endif
    }
    _tokens.insert(Token(time, TimeObject(), task));

    if(_wakeUpTime == TimeObject() || time < _wakeUpTime)
    {
        _monitor.notify();
    }
    return 0;
}

int Timer::scheduleRepeated(const TimerTaskPtr& task, const TimeObject& delay)
{
    Monitor<Mutex>::Lock sync(_monitor);
    if(_destroyed)
    {
#ifdef _NO_EXCEPTION
        TBSYS_LOG(ERROR,"%s","timer destroyed...");
        return -1;
#else
        throw IllegalArgumentException(__FILE__, __LINE__, "timer destroyed");
#endif
    }

    const Token token(TimeObject::now(TimeObject::Monotonic) + delay, delay, task);
    bool inserted = _tasks.insert(make_pair(task, token.scheduledTime)).second;
    if(!inserted)
    {
#ifdef _NO_EXCEPTION
        TBSYS_LOG(ERROR,"%s","task is already schedulded...");
        return -1;
#else
        throw IllegalArgumentException(__FILE__, __LINE__, "task is already schedulded");
#endif
    }
    _tokens.insert(token); 
   
    if(_wakeUpTime == TimeObject() || token.scheduledTime < _wakeUpTime)
    {
        _monitor.notify();
    }
    return 0;
}

bool Timer::cancel(const TimerTaskPtr& task)
{
    Monitor<Mutex>::Lock sync(_monitor);
    if(_destroyed)
    {
        return false;
    }

    map<TimerTaskPtr, TimeObject, TimerTaskCompare>::iterator p = _tasks.find(task);
    if(p == _tasks.end())
    {
        return false;
    }

    _tokens.erase(Token(p->second, TimeObject(), p->first));
    _tasks.erase(p);

    return true;
}

void
Timer::run()
{
    Token token(TimeObject(), TimeObject(), 0);
    while(true)
    {
        {
            Monitor<Mutex>::Lock sync(_monitor);

            if(!_destroyed)
            {
                if(token.delay != TimeObject())
                {
                    map<TimerTaskPtr, TimeObject, TimerTaskCompare>::iterator p = _tasks.find(token.task);
                    if(p != _tasks.end())
                    {
                        token.scheduledTime = TimeObject::now(TimeObject::Monotonic) + token.delay;
                        p->second = token.scheduledTime;
                        _tokens.insert(token);
                    }
                }
                token = Token(TimeObject(), TimeObject(), 0);

                if(_tokens.empty())
                {
                    _wakeUpTime = TimeObject();
                    _monitor.wait();
                }
            }
            
            if(_destroyed)
            {
                break;
            }
           
            while(!_tokens.empty() && !_destroyed)
            {
                const TimeObject now = TimeObject::now(TimeObject::Monotonic);
                const Token& first = *(_tokens.begin());
                if(first.scheduledTime <= now)
                {
                    token = first;
                    _tokens.erase(_tokens.begin());
                    if(token.delay == TimeObject())
                    {
                        _tasks.erase(token.task);
                    }
                    break;
                }
                
                _wakeUpTime = first.scheduledTime;
                _monitor.timedWait(first.scheduledTime - now);
            }

            if(_destroyed)
            {
                break;
            }
        }     

        if(token.task)
        {
            try
            {
                token.task->runTimerTask();
            }
            catch(const std::exception& e)
            {
                cerr << "Timer::run(): uncaught exception:\n" << e.what() << endl;
            } 
            catch(...)
            {
                cerr << "Timer::run(): uncaught exception" << endl;
            }
        }
    }
}
}// end namespace tbutil
