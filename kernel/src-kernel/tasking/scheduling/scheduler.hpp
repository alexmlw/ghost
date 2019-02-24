/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Ghost, a micro-kernel based operating system for the x86 architecture    *
 *  Copyright (C) 2015, Max Schlüssel <lokoxe@gmail.com>                     *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <system/processor_state.hpp>

#include "ghost/stdint.h"
#include <utils/list_entry.hpp>
#include <tasking/thread.hpp>
#include <system/smp/global_recursive_lock.hpp>

typedef g_list_entry<g_thread*> g_task_entry;

/**
 * The scheduler is responsible for determining which task is the next one to
 * be scheduled and to apply the actual switching.
 */
class g_scheduler {
private:
	uint64_t milliseconds;
	uint32_t coreId;
	uint32_t currentRound;

	g_task_entry* run_queue;
	g_task_entry* idle_entry;
	g_task_entry* current_entry;
	g_task_entry* high_priority;

	/**
	 * Performs the actual context switch.
	 *
	 * @param thread
	 * 		the thread to switch to
	 */
	void _applyContextSwitch(g_thread* thread);

	/**
	 * Removes the given thread from its queue.
	 *
	 * @param thread
	 * 		the thread to remove
	 */
	void _remove(g_thread* thread);

	/**
	 * Checks whether the given thread has died. Removes it (and all child threads).
	 *
	 * @param thread
	 * 		the thread to check
	 * @return whether the thread as died
	 */
	bool _checkAliveState(g_thread* thread);

	/**
	 * Finishs the context switch by setting userspace specific values.
	 *
	 * @param thread
	 * 		thread to finish the switch for
	 */
	void _finishSwitch(g_thread* thread);

	/**
	 * Removes a thread from a queue.
	 *
	 * @param queue_head
	 * 		pointer to a queues head
	 * @param thread
	 * 		the thread to remove
	 * @return the thread entry or nullptr if not found
	 */
	g_task_entry* _removeFromQueue(g_task_entry** queue_head, g_thread* thread);

	/**
	 * Prints a deadlock warning for the current thread.
	 */
	void _printDeadlockWarning();

	/**
	 * Checks the thread for any waiting operations, performs these and moves the
	 * thread to the run queue if possible.
	 *
	 * @param thread
	 * 		the thread to check
	 */
	bool _checkWaitingState(g_thread* thread);

	/**
	 * Chooses which thread should next be scheduled.
	 */
	void selectNext();

	g_task_entry* findEntry(g_thread* thread);

public:
	/**
	 * Creates a new scheduler for the given core.
	 *
	 * @param coreId
	 * 		id of the core
	 */
	g_scheduler(uint32_t coreId);

	/**
	 * Saves the processor state to the thread structure that was
	 * last executed and returns a pointer to this thread. If there is
	 * no last thread, scheduling is performed.
	 *
	 * @param statePtr
	 * 		pointer to the CPU structure on the process stack
	 * @return the last executed thread
	 */
	g_thread* save(g_processor_state* statePtr);

	/**
	 * Performs the scheduling. This processes the waiting tasks and then
	 * does the scheduling algorithm to pick the next task.
	 *
	 * @return the next task to execute
	 */
	g_thread* schedule();
	g_thread* schedule(g_thread* thread);

	/**
	 * Adds the given thread to the run queue.
	 *
	 * @param thread
	 * 		the thread to schedule
	 */
	void add(g_thread* thread);

	/**
	 * Moves the given thread to the top of the wait queue so it is
	 * processed as soon as possible.
	 *
	 * @param thread
	 * 		the thread to push
	 */
	void increaseWaitPriority(g_thread* thread);

	/**
	 * Generates a value that is used to representate the load for this
	 * scheduler.
	 *
	 * @return a numeric load value
	 */
	uint32_t calculateLoad();

	/**
	 * @return the thread that was executed last
	 */
	g_thread* lastThread();

	/**
	 * Retrieves the task with the given id.
	 *
	 * @return the thread or nullptr if not existant
	 */
	g_thread* getTaskById(g_tid id);

	/**
	 * Retrieves the task with the given identifier.
	 *
	 * @param identifier
	 * 		the task identifier to find
	 * @return the thread or nullptr if not existant
	 */
	g_thread* getTaskByIdentifier(const char* identifier);

	/**
	 * Sets all threads of the given process to not alive and returns
	 * whether there where any threads that are still alive.
	 *
	 * @param process
	 * 		the process of which all threads shall be killed
	 */
	void remove_threads(g_process* process);

	/**
	 * Is called by the timer interrupt to count time since the scheduler is
	 * running. Increases the millisecond count by the amount of time that each
	 * timer tick takes.
	 */
	void updateMilliseconds();

	/**
	 * @return the number of milliseconds that this scheduler is running.
	 */
	uint64_t getMilliseconds();

	/**
	 * Counts the number of tasks in this scheduler.
	 */
	uint32_t count();

	/**
	 * Writes the IDs of all tasks that this scheduler holds into the given
	 * array.
	 */
	uint32_t get_task_ids(g_tid* out, uint32_t len);

	/**
	 * Puts the given thread in the high priority queue, forcing the scheduler
	 * to process this thread asap.
	 */
	void pushHighPriority(g_thread* thread);


};

#endif
