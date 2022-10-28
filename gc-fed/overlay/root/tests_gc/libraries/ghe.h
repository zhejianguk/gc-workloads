#include <stdint.h>
#include "rocc.h"

#define GHE_FULL 0x02
#define GHE_EMPTY 0x01


static inline uint64_t debug_icounter ()
{
  uint64_t icounter;
  ROCC_INSTRUCTION_D (1, icounter, 0x1a);
  return icounter;
}

static inline uint64_t debug_gcounter ()
{
  uint64_t icounter;
  ROCC_INSTRUCTION_D (1, icounter, 0x23);
  return icounter;
}

static inline uint64_t debug_mcounter ()
{
  uint64_t mcounter;
  ROCC_INSTRUCTION_D (1, mcounter, 0x19);
  return mcounter;
}



static inline uint64_t ghe_status ()
{
  uint64_t status;
  ROCC_INSTRUCTION_D (1, status, 0x00);
  return status; 
  // 0b01: empty; 
  // 0b10: full;
  // 0b00: data buffered;
  // 0b11: error
}


static inline uint64_t ghe_top_func_opcode ()
{
  uint64_t packet = 0x00;
  if (ghe_status() != 0x01) {
    ROCC_INSTRUCTION_D (1, packet, 0x0A);
  }
  return packet;
}

static inline uint64_t ghe_pop_func_opcode ()
{
  uint64_t packet = 0x00;
  if (ghe_status() != 0x01) {
    ROCC_INSTRUCTION_D (1, packet, 0x0B);
  }
  return packet;
}


static inline uint64_t ghe_top_data ()
{
  uint64_t packet = 0x00;
  if (ghe_status() != 0x01) {
    ROCC_INSTRUCTION_D (1, packet, 0x0C);
  }
  return packet;
}


static inline uint64_t ghe_pop_data ()
{
  uint64_t packet = 0x00;
  if (ghe_status() != 0x01) {
    ROCC_INSTRUCTION_D (1, packet, 0x0D);
  }
  return packet;
}


static inline uint64_t ghe_checkght_status ()
{
  uint64_t status;
  ROCC_INSTRUCTION_D (1, status, 0x07);
  return status; 
}


static inline uint64_t ghe_complete ()
{
  uint64_t get_status = 0;
  uint64_t set_status = 0x01;
  ROCC_INSTRUCTION_DS (1, get_status, set_status, 0x01);
  return get_status; 
}

static inline uint64_t ghe_release ()
{
  uint64_t get_status = 0;
  uint64_t set_status = 0xFF;
  ROCC_INSTRUCTION_DS (1, get_status, set_status, 0x01);
  return get_status; 
}

static inline uint64_t ghe_go ()
{
  uint64_t get_status = 0;
  uint64_t set_status = 0;
  ROCC_INSTRUCTION_DS (1, get_status, set_status, 0x01);
  return get_status; 
}

static inline uint64_t ghe_agg_status ()
{
  uint64_t status;
  ROCC_INSTRUCTION_D (1, status, 0x10);
  return status; 
  // 0b01: empty; 
  // 0b10: full;
  // 0b00: data buffered;
  // 0b11: error
}

static inline void ghe_agg_push (uint64_t header, uint64_t payload)
{
  ROCC_INSTRUCTION_SS (1, header, payload, 0x11);
}

static inline uint64_t ghe_sch_status ()
{
  uint64_t status;
  ROCC_INSTRUCTION_D (1, status, 0x20);
  return status; 
  // 0b01: empty; 
  // 0b10: full;
  // 0b00: data buffered;
  // 0b11: error
}

int gc_pthread_setaffinity(uint64_t phart_id){
	pthread_t thread_id = pthread_self();
	cpu_set_t cpu_id;
	int s;

	CPU_ZERO(&cpu_id);
	CPU_SET(phart_id, &cpu_id); 
	s = pthread_setaffinity_np(thread_id, sizeof(cpu_id), &cpu_id);
	
	return s;
} 