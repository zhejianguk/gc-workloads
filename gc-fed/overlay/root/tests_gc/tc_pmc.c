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

int uart_lock;
int if_tasks_initalised[NUM_CORES] = {0};




void* thread_boom(void* args){
	uint64_t hart_id = (uint64_t) args;
	uint64_t proc_id = remapping_hart_id(hart_id);
	

	uint64_t mcounter;
	uint64_t icounter;
	uint64_t gcounter;

	if (gc_pthread_setaffinity(proc_id) != 0){
		lock_acquire(&uart_lock);
		printf ("[Boom-C%x]: pthread_setaffinity failed.", hart_id);
		lock_release(&uart_lock);
	} else{
		ght_set_satp_priv ();
		if_tasks_initalised[proc_id] = 1;
	}

	//================== Initialisation ==================//
	while (and_gate(if_tasks_initalised, NUM_CORES) == 0){
	}
	
	ghm_cfg_agg(0x04);

	// Insepct load operations 
	// index: 0x01 
	// Func: 0x00; 0x01; 0x02; 0x03; 0x04; 0x05
	// Opcode: 0x03
	// Data path: N/U
	ght_cfg_filter(0x01, 0x00, 0x03, 0x02); // lb
	ght_cfg_filter(0x01, 0x01, 0x03, 0x02); // lh
	ght_cfg_filter(0x01, 0x02, 0x03, 0x02); // lw
	ght_cfg_filter(0x01, 0x03, 0x03, 0x02); // ld
	ght_cfg_filter(0x01, 0x04, 0x03, 0x02); // lbu
	ght_cfg_filter(0x01, 0x05, 0x03, 0x02); // lhu

	// se: 00, end_id: 0x01, scheduling: rr, start_id: 0x01
  	ght_cfg_se (0x00, 0x01, 0x01, 0x01);
	// se: 01, end_id: 0x03, scheduling: rr, start_id: 0x02
	ght_cfg_se (0x01, 0x03, 0x01, 0x02);
	
	ght_cfg_mapper (0x01, 0b0111);
	
	lock_acquire(&uart_lock);
	printf("[Boom-%x]: Test is now started: \r\n", hart_id);
	lock_release(&uart_lock);
	ght_set_status (0x01); // ght: start

	//===================== Execution =====================//
	int sum_temp = 0;
	int sum = 0;
	for (int i = 0; i < 17000; i++ ){
    	sum_temp = task_synthetic_malloc(i);
    	sum = sum + sum_temp;
	}

	//=================== Post execution ===================//
	uint64_t status;
	ght_set_status (0x02);

	while (((status = ght_get_status()) < 0x1FFFF) || (ght_get_buffer_status() != GHT_EMPTY)){

	}
	mcounter = debug_mcounter();
  	icounter = debug_icounter();
  	gcounter = debug_gcounter();

	lock_acquire(&uart_lock);
	printf("[Boom-%x]: All tests are done! Status: %x, Sum: %x \r\n", hart_id, status, sum);
	lock_release(&uart_lock);

  	lock_acquire(&uart_lock);
  	printf("Debug, m-counter: %x \r\n", mcounter);
  	printf("Debug, i-counter: %x \r\n", icounter);
  	printf("Debug, g-counter: %x \r\n", gcounter);
  	lock_release(&uart_lock);


	ght_unset_satp_priv();
	ght_set_status (0x00);
	
	return NULL;
}

void* thread_pmc(void* args){
	uint64_t hart_id = (uint64_t) args;
	uint64_t proc_id = remapping_hart_id(hart_id);

	// GC variables
	uint64_t perfc = 0;
	uint64_t Payload = 0x0;
	uint64_t ecounter;

	if (gc_pthread_setaffinity(proc_id) != 0){
		lock_acquire(&uart_lock);
		printf ("[Boom-C%x]: pthread_setaffinity failed.", hart_id);
		lock_release(&uart_lock);
	} else{
		ghe_go();
		if_tasks_initalised[proc_id] = 1;
	}

	//================== Initialisation ==================//
	while (and_gate(if_tasks_initalised, NUM_CORES) == 0){

	}
	//===================== Execution =====================// 
	while (ghe_checkght_status() == 0x00){
	}
	
	while ((ghe_checkght_status() != 0x02) || (ghe_status() != GHE_EMPTY)) {
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);
			perfc = perfc + 1;
		}
	}
	//=================== Post execution ===================//
	ecounter = debug_ecounter();

	lock_acquire(&uart_lock);
	printf("[Rocket-C%x-PMC]: Completed, PMC = %x! Debug -- E-counter: %x \r\n", hart_id, perfc, ecounter);
	lock_release(&uart_lock);

	ghe_release();

	return NULL;
}

int main(){
	pthread_t threads[NUM_CORES];

	pthread_create(&threads[0], NULL, thread_boom, (void *) 0);	

	for (uint64_t i = 1; i < NUM_CORES; i++) {
		pthread_create(&threads[i], NULL, thread_pmc, (void *) i);
	}
	
	for (uint64_t i = 0; i < NUM_CORES; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}