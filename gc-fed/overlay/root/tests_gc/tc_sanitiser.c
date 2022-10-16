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
#include "malloc.h"

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
		printf ("[Boom]: pthread_setaffinity failed.");
	} else {
		if_tasks_initalised[hart_id] = 1;
	}

	while (or_gate(if_tasks_initalised, NUM_CORES) == 0){
	}

	// Insepct load operations 
	// index: 0x01 
	// Func: 0x00; 0x01; 0x02; 0x03; 0x04; 0x05
	// Opcode: 0x03
	// Data path: MemCaluc
	ght_cfg_filter(0x01, 0x00, 0x03, 0x02); // lb
	ght_cfg_filter(0x01, 0x01, 0x03, 0x02); // lh
	ght_cfg_filter(0x01, 0x02, 0x03, 0x02); // lw 
	ght_cfg_filter(0x01, 0x03, 0x03, 0x02); // ld
	ght_cfg_filter(0x01, 0x04, 0x03, 0x02); // lbu
	ght_cfg_filter(0x01, 0x05, 0x03, 0x02); // lhu

	// Insepct store operations 
	// index: 0x02 
	// Func: 0x00; 0x01; 0x02; 0x03
	// Opcode: 0x23
	// Data path: MemCaluc
	ght_cfg_filter(0x02, 0x00, 0x23, 0x03); // sb
	ght_cfg_filter(0x02, 0x01, 0x23, 0x03); // sh
	ght_cfg_filter(0x02, 0x02, 0x23, 0x03); // sw
	ght_cfg_filter(0x02, 0x03, 0x23, 0x03); // sd

	// se: 01, end_id: 0x02, scheduling: rr, start_id: 0x01
	ght_cfg_se (0x01, 0x02, 0x01, 0x01);

	// se: 02, end_id: 0x03, scheduling: rr, start_id: 0x03
	ght_cfg_se (0x02, 0x03, 0x01, 0x03);

	ght_cfg_mapper (0x01, 0b0110);
	ght_cfg_mapper (0x02, 0b0110);

	lock_acquire(&uart_lock);
	printf("[Boom-%x]: Test is now started: \r\n", hart_id);
	lock_release(&uart_lock);
	ght_set_status (0x04); // ght: record PTBR
	ght_set_status (0x01); // start monitoring
	//===================== Execution =====================//
	printf("Hello Guardian-Council !! \r\n");

	int sum_temp = 0;
	int sum = 0;
	for (int i = 0; i < 170; i++ ){
    	sum_temp = task_synthetic_malloc(i);
    	sum = sum + sum_temp;
    	printf("");
	}

	//=================== Post execution ===================//
	uint64_t status;

	ght_set_status (0x02);
	while (((status = ght_get_status()) < 0x1FFFF) || (ght_get_buffer_status() != GHT_EMPTY)){
	}

	printf("[Boom-%x]: All tests are done! Status: %x; Sum: %x -- addr: %x \r\n", hart_id, status, sum, &sum);
	
	// shadow memory
	shadow_free(shadow);
	return NULL;
}

void* thread_sanitiser(void* args){
	
	pthread_t thread_id = pthread_self();
	pid_t pid;
	cpu_set_t cpu_id;
	int s;
	uint64_t hart_id = (uint64_t) args;

	// GC variables
	uint64_t Address = 0x0;
  	uint64_t Err_Cnt = 0x0;
	

	//================== Initialisation ==================//
	CPU_ZERO(&cpu_id);
	CPU_SET(hart_id, &cpu_id); 
	s = pthread_setaffinity_np(thread_id, sizeof(cpu_id), &cpu_id);
	if (s != 0) {
		printf ("[Rocket-C%x]: pthread_setaffinity failed.", hart_id);
	} else {
		if_tasks_initalised[hart_id] = 1;
	}

	while (or_gate(if_tasks_initalised, NUM_CORES) == 0){
	}

	while (ghe_checkght_status() == 0x00){
	};
	//===================== Execution =====================// 
	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
		uint64_t Header = 0x0;
		uint64_t PC = 0x0;
		uint64_t Inst = 0x0;
		ROCC_INSTRUCTION_D (1, Header, 0x0A);    
		ROCC_INSTRUCTION_D (1, Address, 0x0D);

		asm volatile("fence rw, rw;");

		PC = Header >> 32;
		Inst = Header & 0xFFFFFFFF;

		char bits = shadow[(Address)>>7];
		
		if(bits & (1<<((Address&0x70)>>4))) {
			lock_acquire(&uart_lock);
			printf("[Rocket-C%x-Sani]: **Error** illegal accesses at %x, PC: %x, Inst: %x. \r\n", hart_id, Address, PC, Inst);
			lock_release(&uart_lock);
			Err_Cnt ++;
			// return -1;
		}
		}


		if ((ghe_status() == GHE_EMPTY) && (ghe_checkght_status() == 0x00)) {
		ghe_complete();
		while((ghe_checkght_status() == 0x00)) {
			// Wait big core to re-start
		}
		ghe_go();
		}
	}

	//=================== Post execution ===================//
	printf("Rocket-C%x-Sani]: Completed, %x illegal accesses are detected.\r\n", hart_id, Err_Cnt);
	ghe_release();
	
	return 0;
}

int main(){
	pthread_t threads[NUM_CORES];

	// shadow memory
	shadow = shadow_malloc(32*1024*1024*sizeof(char));
  	if(shadow == NULL) {
    	printf("[Boom]: Error! memory not allocated.");
    	exit(0);
  	} 


	pthread_create(&threads[0], NULL, thread_boom, (void *) 0);	


	for (uint64_t i = 1; i < NUM_CORES; i++) {
		pthread_create(&threads[i], NULL, thread_sanitiser, (void *) i);
	}
	
	for (uint64_t i = 0; i < NUM_CORES; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}