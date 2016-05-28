/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;
static eos_tcb_t test_task[2];

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);
//	static int num_task = 0;
		task->_StkPtr = _os_create_context(sblock_start, sblock_size, entry, arg);
		task->_taskState = READY;
		task->_priority = priority;
		task->_period = 0;
		task->_next_period = 0;
	if(priority != LOWEST_PRIORITY){
		ready_enqueue(task);
		_os_set_ready(priority);
//		num_task++;
	}
	return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

void ready_enqueue(eos_tcb_t *task) {
	int32u_t priority = task->_priority;
	_os_node_t *temp = (struct _os_node_t *)malloc(sizeof(struct _os_node_t));
	temp->previous = NULL;
	temp->next = NULL;
	temp->_priority = priority;
	temp->ptr_data = task;

	if (_os_ready_queue[priority] == NULL)
		_os_ready_queue[priority] = temp;
	else {
		_os_node_t *cur = _os_ready_queue[priority];
		while (cur->next != NULL) cur = cur->next;
		cur->next = temp;
		temp->previous = cur;
	}
}
eos_tcb_t* ready_dequeue(int32u_t priority) {
	if (_os_ready_queue[priority] == NULL) {
		return 0;
	}
	else {
		_os_node_t *temp = _os_ready_qeue[priority];
		eos_tcb_t *ret = _os_ready_queue[priority]->ptr_data;
		if (_os_ready_queue[priority]->next != NULL) {
			_os_ready_queue[priority] = _os_ready_queue[priority]->next;
			_os_ready_queue[priority]->previous = NULL;
		}
		else {
			_os_ready_queue[priority] = NULL;
		}
		_os_unset_ready(priority);
		free(temp);
		return ret;
	}
}
void eos_schedule() {			
//	static int task_init = 0;	// boolean for determining any task is not called
	if(_os_current_task != 0){		// if there is a task running
		
		addr_t tmp_addr;
		tmp_addr = _os_save_context();		// save current task and
		if(tmp_addr != NULL){		// if return value is not NULL,
			int32u_t h_prior = _os_get_highest_priority();
			eos_tcb_t *next_task = ready_dequeue[h_prior];
			_os_current_task->_StkPtr = tmp_addr;
			_os_current_task->_taskState = WAITING;
			tmp_addr = next_task->_StkPtr;
			next_task->_taskState = RUNNING;
			_os_current_task = next_task;
			/*
			if(test_task[0]._taskState == RUNNING){
				test_task[0]._StkPtr = tmp_addr;		// change current tcb's stack pointer to the returned value
				test_task[0]._taskState = WAITING;		
				tmp_addr = test_task[1]._StkPtr;
				test_task[1]._taskState = RUNNING;
			//PRINT("sgfg\n");
			}
			else {
				test_task[1]._StkPtr = tmp_addr;
				test_task[1]._taskState = WAITING;	
				tmp_addr = test_task[0]._StkPtr;
				test_task[0]._taskState = RUNNING;
			}
			*/
			sleep(0);
			_os_restore_context(tmp_addr);
		}
		else{
		}
	}
	else{		// there is not any task running now
//		task_init = 1;
		int32u_t h_prior = _os_get_highest_priority();
		eos_tcb_t *next_task = ready_dequeue[h_prior];
		*_os_current_task = next_task;
		next_task->_taskState = RUNNING;
		_os_restore_context(next_task->_StkPtr);
	}

}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
	task->_period = period;
	task->_next_period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
	eos_counter_t *counter = eos_get_system_timer();
	eos_alarm_t *sleep_alarm = (struct eos_alarm_t *)malloc(sizeof(struct eos_alarm_t));
	int32u_t timeout = _os_current_task->_period + counter->tick;
	eos_set_alarm(counter, sleep_alarm, timeout, _os_wakeup_sleeping_task, _os_current_task);
	eos_schedule();
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
	eos_tcb_t *sleeping_task = arg;
	sleeping_task->_taskState = READY;
	ready_enqueue(sleeping_task);
	eos_schedule();
}

