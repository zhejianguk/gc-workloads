#include <stdio.h>
#include <stdlib.h>
#include "rocc.h"

#define TRUE 0x01
#define FALSE 0x00
#define NUM_CORES 8

#define GHT_FULL 0x02
#define GHT_EMPTY 0x01

static inline uint64_t remapping_hart_id (uint64_t hart_id)
{
  uint64_t process_id;
  switch(hart_id) {
	case 0:
    process_id = 0;
    break;
	case 1:
    process_id = 1;
    break;
	case 2:
    process_id = 2;
    break;
	default:
    process_id = hart_id;
		break;
  }

  return process_id;
}


static inline uint64_t debug_ecounter ()
{
  uint64_t ecounter;
  ROCC_INSTRUCTION_D (1, ecounter, 0x22);
  return ecounter;
}

static inline void ght_set_status (uint64_t status)
{
  asm volatile("fence rw, rw;");
  ROCC_INSTRUCTION_SS (1, status, 0X01, 0x06);
  asm volatile("fence rw, rw;");
}

static inline uint64_t ght_get_status ()
{
  uint64_t get_status;
  ROCC_INSTRUCTION_DSS (1, get_status, 0X00, 0X00, 0x06);
  return get_status;
}

static inline uint64_t ght_get_satp ()
{
  uint64_t get_satp;
  ROCC_INSTRUCTION_DSS (1, get_satp, 0X00, 0X00, 0x17);
  return get_satp;
}

static inline uint64_t ght_get_priv ()
{
  uint64_t get_priv;
  ROCC_INSTRUCTION_DSS (1, get_priv, 0X00, 0X00, 0x18);
  return get_priv;
}

static inline void ght_set_satp_priv ()
 {
   ROCC_INSTRUCTION_S (1, 0x01, 0x16);
 }

 static inline void ght_unset_satp_priv ()
 {
   ROCC_INSTRUCTION_S (1, 0x02, 0x16);
 }

static inline uint64_t ght_get_buffer_status ()
{
  uint64_t get_buffer_status;
  ROCC_INSTRUCTION_DSS (1, get_buffer_status, 0X00, 0X00, 0x08);
  return get_buffer_status;
}

static inline void ght_cfg_filter (uint64_t index, uint64_t func, uint64_t opcode, uint64_t sel_d)
{
  uint64_t set_ref;
  set_ref = ((index & 0x1f)<<4) | ((sel_d & 0xf)<<17) | ((opcode & 0x7f)<<21) | ((func & 0xf)<<28) | 0x02;
  ROCC_INSTRUCTION_SS (1, set_ref, 0X02, 0x06);
}

static inline void ght_cfg_se (uint64_t se_id, uint64_t end_id, uint64_t policy, uint64_t start_id)
{
  uint64_t set_se;
  set_se = ((se_id & 0x1f)<<4) | ((start_id & 0xf)<<17) | ((policy & 0x7f)<<21) | ((end_id & 0xf)<<28) | 0x04;
  ROCC_INSTRUCTION_SS (1, set_se, 0X02, 0x06);
}

static inline void ght_cfg_mapper (uint64_t inst_type, uint64_t ses_receiving_inst)
{
  uint64_t set_mapper;
  set_mapper = ((inst_type & 0x1f)<<4) | ((ses_receiving_inst & 0xFFFF)<<16) | 0x03;
  ROCC_INSTRUCTION_SS (1, set_mapper, 0X02, 0x06);
}

static inline void ghm_cfg_agg (uint64_t agg_core_id)
{
  uint64_t agg_core_set;
  agg_core_set = ((agg_core_id & 0xffff)<<16) | 0x08;
  ROCC_INSTRUCTION_SS (1, agg_core_set, 0X02, 0x06);
}

void idle()
{
  while(1){};
}

uint64_t task_synthetic_malloc (uint64_t base)
{
  uint64_t *ptr = NULL;
  uint64_t ptr_size = 32;
  uint64_t sum = 0;
    
  ptr = (uint64_t*) malloc(ptr_size * sizeof(uint64_t));


  // if memory cannot be allocated
  if(ptr == NULL) {
    printf("Error! memory not allocated. \r\n");
    exit(0);
  }

  for (uint64_t i = 0; i < ptr_size; i++)
  {
    *(ptr + i) = base + i;
    *(ptr + i) = *(ptr + i) + *(ptr + ptr_size - i - 1);
  }

  for (uint64_t i = 0; i < ptr_size; i++)
  {
    sum = sum + *(ptr+i);
  }

  free(ptr);

  return sum;
}

