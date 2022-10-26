#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include "libraries/spin_lock.h"
#include "libraries/ght.h"
#include "libraries/ghe.h"
#include "libraries/deque.h"

char* shadow;
int uart_lock;
int if_tasks_initalised[NUM_CORES] = {0};

void* thread_boom(void* args){
	pthread_t thread_id = pthread_self();
	cpu_set_t cpu_id;
	uint64_t hart_id = (uint64_t) args;
	int s;
	
	//================== Initialisation ==================//
	CPU_ZERO(&cpu_id);
	CPU_SET(hart_id, &cpu_id); 
	s = pthread_setaffinity_np(thread_id, sizeof(cpu_id), &cpu_id);
	if (s != 0) {
		lock_acquire(&uart_lock);
		printf ("[Boom]: pthread_setaffinity failed.");
		lock_release(&uart_lock);
	} else {
		ght_set_satp_priv ();
		if_tasks_initalised[hart_id] = 1;
	}

	while (and_gate(if_tasks_initalised, NUM_CORES) == 0){
	}

	// Insepct JAL 
	// inst_index: 0x03 
	// Func: 0x00 - 0x07
	// Opcode: 0x6F
	// Data path: ALU + JALR
	ght_cfg_filter(0x03, 0x00, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x01, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x02, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x03, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x04, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x05, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x06, 0x6F, 0x04);
	ght_cfg_filter(0x03, 0x07, 0x6F, 0x04);

	// Insepct JALR 
	// inst_index: 0x03 
	// Func: 0x00
	// Opcode: 0x67
	// Data path: ALU + JALR
	ght_cfg_filter(0x03, 0x00, 0x67, 0x04);

	// Insepct Special RET 
	// inst_index: 0x03
	// Func: 0x00
	// Opcode: 0x02
	// Data path: ALU + JALR
	ght_cfg_filter(0x03, 0x00, 0x02, 0x04);

	// Insepct Special JALR 
	// inst_index: 0x03
	// Func: 0x01
	// Opcode: 0x02
	// Data path: ALU + JALR
	ght_cfg_filter(0x03, 0x01, 0x02, 0x04);

	// se: 0x03, end_id: 0x01, scheduling: rr, start_id: 0x01
	ght_cfg_se (0x03, 0x01, 0x01, 0x01);

	// inst_index: 0x03 se: 0x03
	ght_cfg_mapper (0x03, 0b1000);


	lock_acquire(&uart_lock);
	printf("[Boom-%x]: Test is now started: \r\n", hart_id);
	lock_release(&uart_lock);
	ght_set_status (0x01); // start monitoring

	//===================== Execution =====================//
	lock_acquire(&uart_lock);
	printf("Hello Guardian-Council !! \r\n");
	lock_release(&uart_lock);

	int sum_temp = 0;
	int sum = 0;
	for (int i = 0; i < 170; i++ ){
    	sum_temp = task_synthetic_malloc(i);
    	sum = sum + sum_temp;
	}

	//=================== Post execution ===================//
	ght_set_status (0x02);
	uint64_t status;
	while (((status = ght_get_status()) < 0x1FFFF) || (ght_get_buffer_status() != GHT_EMPTY)){
	}


	lock_acquire(&uart_lock);
	printf("[Boom-%x]: All tests are done! Status: %x; Sum: %x -- addr: %x \r\n", hart_id, status, sum, &sum);
	lock_release(&uart_lock);

	ght_unset_satp_priv ();
	ght_set_status (0x00);
	return NULL;
}

void* thread_shadowstack_s(void* args){
	
	pthread_t thread_id = pthread_self();
	pid_t pid;
	cpu_set_t cpu_id;
	int s;
	uint64_t hart_id = (uint64_t) args;

	// GC variables
	uint64_t Header = 0x0;  
	uint64_t Opcode = 0x0;
	uint64_t Func = 0x0;
	uint64_t Rd = 0x0;
	uint64_t RS1 = 0x0;
	uint64_t Payload = 0x0;
	uint64_t PC = 0x0;
	uint64_t Inst = 0x0;
	uint64_t PayloadPush = 0x0;
	uint64_t PayloadPull = 0x0;

	dequeue  shadow_header;
 	dequeue  shadow_payload;
  	initialize(&shadow_header);
  	initialize(&shadow_payload);

	//================== Initialisation ==================//
	CPU_ZERO(&cpu_id);
	CPU_SET(hart_id, &cpu_id); 
	s = pthread_setaffinity_np(thread_id, sizeof(cpu_id), &cpu_id);
	if (s != 0) {
		lock_acquire(&uart_lock);
		printf ("[Rocket-C%x]: pthread_setaffinity failed.", hart_id);
		lock_release(&uart_lock);
	} else {
		ghe_go();
		if_tasks_initalised[hart_id] = 1;
	}

	while (and_gate(if_tasks_initalised, NUM_CORES) == 0){
	}


	while (ghe_checkght_status() == 0x00){
	};

	//===================== Execution =====================// 
	 while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Header, 0x0A);
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);

			Opcode = Header & 0x7F;
			Func = (Header & 0x7000) >> 12;
			Rd = (Header & 0xF80) >> 7;
			RS1 = (Header & 0xF8000) >> 15;
			PC = Header >> 32;
			Inst = Header & 0xFFFFFFFF;
		
			// Push -- a function is called
			if ((((Opcode == 0x6F) || (Opcode == 0x67)) && (Rd == 0x01)) || // + comprised inst
				((Opcode == 0x02) && (Func == 0x01) && (Rd != 0x00) && ((RS1 & 0x01) == 0X01))) {
				PayloadPush = Payload;
				if (full(&shadow_payload) == 0) {
					enqueueF(&shadow_header, Header);
					enqueueF(&shadow_payload, PayloadPush);
					lock_acquire(&uart_lock);
					printf("[C%x SS]: <<Pushed>> Expected: %x.                        PC: %x. Inst: %x. \r\n", hart_id, PayloadPush, PC, Inst);
					lock_release(&uart_lock);
				}
			}

			// Pull -- a function is returned
			if (((Opcode == 0x67) && (Rd == 0x00)) || // + comprised inst
				((Opcode == 0X02) && (Func == 0x00 ) && (Rd == 0x01) && ((RS1 & 0x01) == 0X01))) {
				PayloadPull = Payload;
				if (empty(&shadow_payload) == 1) {
					lock_acquire(&uart_lock);
					printf("[C%x SS]: ==Empty== Uninteded: %x.                        PC: %x. Inst: %x. \r\n", hart_id, PayloadPull, PC, Inst);
					lock_release(&uart_lock);
				} else {
					u_int64_t comp = dequeueF(&shadow_payload);
					dequeueF(&shadow_header);
				
					if (comp != PayloadPull){
						lock_acquire(&uart_lock);
						printf("[C%x SS]: **Error**  Expected: %x. v.s. Pulled: %x. PC: %x. Inst: %x. \r\n", hart_id, comp, PayloadPull, PC, Inst);
						lock_release(&uart_lock);
						// return -1;
					} else {
						lock_acquire(&uart_lock);
						printf("[C%x SS]: --Paried-- Expected: %x. v.s. Pulled: %x. PC: %x. Inst: %x. \r\n", hart_id, comp, PayloadPull, PC, Inst);
						lock_release(&uart_lock);
					}
				}
			}
		}

		/*
		if ((ghe_status() == GHE_EMPTY) && (ghe_checkght_status() == 0x00)) {
			ghe_complete();
			while((ghe_checkght_status() == 0x00)) {
			// Wait big core to re-start
			}
			ghe_go();
		}
		*/
	}

	//=================== Post execution ===================//
	if (hart_id == 1){
    	lock_acquire(&uart_lock);
    	printf("[C%x SS]: Completed. No error is detected\r\n", hart_id);
   		lock_release(&uart_lock);
	}
 	ghe_release();
	return NULL;
}

int main(){
	pthread_t threads[NUM_CORES];

	pthread_create(&threads[0], NULL, thread_boom, (void *) 0);	


	for (uint64_t i = 1; i < NUM_CORES; i++) {
		pthread_create(&threads[i], NULL, thread_shadowstack_s, (void *) i);
	}
	
	for (uint64_t i = 0; i < NUM_CORES; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}