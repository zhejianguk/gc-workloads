#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include "spin_lock.h"
#include "ght.h"

int uart_lock;

void* thread_boom(void* args){
	pthread_t thread_id = pthread_self();
	pid_t pid;
	cpu_set_t cpu_id;
	int s;

	uint64_t satp = 0;
	uint64_t priv = 0;

  	int sum_temp = 0;
  	int sum = 0;

	CPU_ZERO(&cpu_id);
	CPU_SET(remapping_hart_id(0), &cpu_id); 
	s = pthread_setaffinity_np(thread_id, sizeof(cpu_id), &cpu_id);
	if (s != 0) {
		printf ("[Boom]: pthread_setaffinity failed.");
	} else { 	
		printf ("[Boom]: Bounded Thread ID: %x to CPU: 0. \r\n", thread_id);
	}

	pid = gettid ();
	printf ("[Boom]: Hello world, from Thread ID: %x; PID: %d. \r\n", thread_id, pid);

	lock_acquire(&uart_lock);
  	printf("[Boom]: Test is now started: \r\n");
  	lock_release(&uart_lock);
	ght_set_satp_priv ();
  	ght_set_status (0x01); // ght: start
	

	satp = ght_get_satp();
  	priv = ght_get_priv();
  	lock_acquire(&uart_lock);
  	printf("[Boom]: PTBR = %x; PRIV= %x \r\n", satp, priv);
  	lock_release(&uart_lock);
	
	
	while(1) {

	}
	return NULL;
}

void* thread_checker(void* args){
	pthread_t thread_id = pthread_self();
	pid_t pid;
	cpu_set_t cpu_id;
	int s;	

	CPU_ZERO(&cpu_id);
	CPU_SET(remapping_hart_id(3), &cpu_id); 
	s = pthread_setaffinity_np(thread_id, sizeof(cpu_id), &cpu_id);
	if (s != 0) {
		printf ("[Rocket]: pthread_setaffinity failed.");
	} else {
		printf ("[Rocket]: Bounded Thread ID: %x to CPU: 3. \r\n", thread_id);
	}

	pid = gettid ();
	printf ("[Rocket]: Hello world, from Thread ID: %x; PID: %d. \r\n", thread_id, pid);
	while(1) {

	}
	return NULL;
}

int main(){
	pthread_t thread_1;
	pthread_t thread_2;
	

	pthread_create(&thread_1, NULL, thread_boom, NULL);	
	pthread_create(&thread_2, NULL, thread_checker, NULL);
	
	pthread_join(thread_1, NULL);	
	pthread_join(thread_2, NULL);
	return 0;
}