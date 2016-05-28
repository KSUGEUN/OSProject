/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: 
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

void eos_set_alarm(eos_counter_t* counter, eos_alarm_t* alarm, int32u_t timeout, void (*entry)(void *arg), void *arg) {
	//2.
	if (timeout == 0 || entry == NULL) return;
	//3.
	alarm->timeout = timeout;
	alarm->handler = entry;
	alarm->arg = arg;
	alarm->alarm_queue_node.ptr_data = alarm;
	//4.
	_os_node_t *cursor = counter->alarm_queue;
	if (cursor == NULL) counter->alarm_queue = &alarm->alarm_queue_node;	// 처음일 경우
	else {
		while (cursor->next != NULL && (cursor->ptr_data->timeout < timeout)) cursor = cursor->next;	// 끝 node or 자신보다 timeout이 큰 경우
		if (cursor->next == NULL && cursor->ptr_data->timeout < timeout) {	// 끝 node이고 자신보다 timeout이 작을때
			cursor->next = &alarm->alarm_queue_node;
			alarm->alarm_queue_node.previous = cursor;
		}
		else {	//끝 node이나 timeout이 더 클 때 or 중간일 경우
				_os_node_t *temp = cursor;
				cursor = cursor->previous;
				cursor->next = &alarm->alarm_queue_node;
				alarm->alarm_queue_node.previous = cursor;
				alarm->alarm_queue_node.next = temp;
				temp->previous = &alarm->alarm_queue_node;
		}
	}
}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
	//1. 카운터의 tick을 1 증가시킴
	counter->tick += 1;
	//2.
	if (counter->alarm_queue->ptr_data->timeout == tick) {
		if (counter->alarm_queue->ptr_data->handler != NULL)
			counter->alarm_queue->ptr_data->handler();
		if(counter->alarm_queue->next != NULL)
		counter->alarm_queue = counter->alarm_queue->next;
		counter->alarm_queue->previous = NULL;
	}
	PRINT("tick\n");
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
