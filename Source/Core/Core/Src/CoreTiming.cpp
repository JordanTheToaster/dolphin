// Copyright (C) 2003-2008 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "Thread.h"
#include "PowerPC/PowerPC.h"
#include "CoreTiming.h"

// TODO(ector): Replace new/delete in this file with a simple memory pool
// Don't expect a massive speedup though.

namespace CoreTiming
{
int downcount, slicelength;
int maxSliceLength = 20000;

s64 globalTimer;
s64 idledCycles;

u64 GetTicks()
{
    return (u64)globalTimer; 
}

u64 GetIdleTicks()
{
	return (u64)idledCycles;
}

struct Event
{
	TimedCallback callback;
	Event *next;
	s64 time;
	u64 userdata;
	const char *name;
};

Event *first;
Event *tsFirst;

Common::CriticalSection externalEventSection;

// This is to be called when outside threads, such as the graphics thread, wants to
// schedule things to be executed on the main thread.
void ScheduleEvent_Threadsafe(int cyclesIntoFuture, TimedCallback callback, const char *name, u64 userdata)
{
	externalEventSection.Enter();
	Event *ne = new Event;
	ne->time = globalTimer + cyclesIntoFuture;
	ne->name = name;
	ne->callback = callback;
	ne->next = tsFirst;
	ne->userdata = userdata;
	tsFirst = ne;
	externalEventSection.Leave();
}

void Clear()
{
	globalTimer = 0;
	idledCycles = 0;
	while (first)
	{
		Event *e = first->next;
		delete [] first;
		first = e;
	}
}

void AddEventToQueue(Event *ne)
{
	// Damn, this logic got complicated. Must be an easier way.
	if (!first)
	{
		first = ne;
		ne->next = 0;
	}
	else
	{
		Event *ptr = first;
		Event *prev = 0;
		if (ptr->time > ne->time)
		{
			ne->next = first;
			first = ne;
		}
		else
		{
			prev = first;
			ptr = first->next;

			while (ptr)
			{
				if (ptr->time <= ne->time)
				{
					prev = ptr;
					ptr = ptr->next;
				}
				else
					break;
			}

			//OK, ptr points to the item AFTER our new item. Let's insert
			ne->next = prev->next;
			prev->next = ne;
			// Done!
		}
	}
}

// This must be run ONLY from within the cpu thread
// cyclesIntoFuture may be VERY inaccurate if called from anything else
// than Advance 
void ScheduleEvent(int cyclesIntoFuture, TimedCallback callback, const char *name, u64 userdata)
{
	Event *ne = new Event;
	ne->callback = callback;
	ne->userdata = userdata;
	ne->name = name;
	ne->time = globalTimer + cyclesIntoFuture;

	AddEventToQueue(ne);
}

void (*advanceCallback)(int cyclesExecuted);


void RegisterAdvanceCallback(void (*callback)(int cyclesExecuted))
{
	advanceCallback = callback;
}

bool IsScheduled(TimedCallback callback) 
{
	if (!first)
		return false;
	Event *e = first;
	while (e) {
		if (e->callback == callback)
			return true;
		e = e->next;
	}
	return false;
}

void RemoveEvent(TimedCallback callback)
{
	if (!first)
		return;
	if (first->callback == callback)
	{
		Event *next = first->next;
		delete first;
		first = next;
	}
	if (!first)
		return;
	Event *prev = first;
	Event *ptr = prev->next;
	while (ptr)
	{
		if (ptr->callback == callback)
		{
			prev->next = ptr->next;
			delete ptr;
			ptr = prev->next;
		}
		else
		{
			prev = ptr;
			ptr = ptr->next;
		}
	}
}

void SetMaximumSlice(int maximumSliceLength)
{
	maxSliceLength = maximumSliceLength;
}


void Advance()
{
	// Move events from async queue into main queue
	externalEventSection.Enter();
	while (tsFirst)
	{
		//MessageBox(0,"yay",0,0);
		Event *next = tsFirst->next;
		AddEventToQueue(tsFirst);
		tsFirst = next;
	}
	externalEventSection.Leave();

	// we are out of run, downcount = -3
	int cyclesExecuted = slicelength - downcount;
	//  sliceLength = downac

	globalTimer += cyclesExecuted;

	while (first)
	{
		if (first->time <= globalTimer)
		{
//			LOG(GEKKO, "[Scheduler] %s     (%lld, %lld) ", 
//				first->name ? first->name : "?", (u64)globalTimer, (u64)first->time);
			
			first->callback(first->userdata, (int)(globalTimer - first->time));

			Event *next = first->next;
			delete first;
			first = next;
		}
		else
		{
			break;
		}
	}
	if (!first) 
	{
		LOG(GEKKO, "WARNING - no events in queue. Setting downcount to 10000");
		downcount += 10000;
	}
	else
	{
		slicelength = (int)(first->time - globalTimer);
		if (slicelength > maxSliceLength)
			slicelength = maxSliceLength;
		downcount = slicelength;
	}
	if (advanceCallback)
		advanceCallback(cyclesExecuted);
}

void LogPendingEvents()
{
	Event *ptr = first;
	while (ptr)
	{
		LOG(GEKKO, "PENDING: Now: %lld Pending: %lld %s", globalTimer, ptr->time, ptr->name);
		ptr = ptr->next;
	}
}

void Idle()
{
	LOG(GEKKO, "Idle");
	
	idledCycles += downcount;
	downcount = 0;
	
	Advance();
}


}; // end of namespace
