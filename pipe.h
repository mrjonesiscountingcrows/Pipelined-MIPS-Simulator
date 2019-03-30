#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

#define MIPS_REGS 32

typedef struct CPU_State_Struct {
  uint32_t PC;		/* program counter */
  int32_t REGS[MIPS_REGS]; /* register file. */
  int FLAG_N;        /* negative flag or sign flag*/
  int FLAG_Z;        /* zero flag */
  int FLAG_V;        /* overflow flag */
  int FLAG_C;        /* carry flag */
} CPU_State;

// Structure to hold Instruction

typedef struct IF
{
uint32_t instruction; 
uint32_t PC;
} IF;

//Structure for Fetch and Decode Stage. No use for this structure

/*typedef struct IF_ID
{
	uint32_t opcode;
	uint32_t RS;
	uint32_t RT;
	uint32_t RD;
	uint32_t shamt;
	uint32_t funct;
	uint32_t immediate;
	uint32_t address;
	int memRead;
	int memWrite;
	int ALUsrc;
	int ALUCTRL;
	int MemtoReg;
	int RegWrite;
	int RegDst;
	int PCSrc;
	int branch;
} buffer_de;

*/

//Structure for Decode and Execute Stage

typedef struct ID_EX
{
	uint32_t opcode;
	int memRead;
	int memWrite;
	int ALUsrc;
	int ALUCTRL;
	int MemtoReg;
	int RegWrite;
	int RegDst;
    int RegDesNumber;
	int PCSrc;
	int branch;
	int shamt;
	uint32_t funct;
	uint32_t PC;
	uint32_t RS;
	uint32_t RT;
    uint32_t reg1;
    uint32_t reg2;
	uint32_t RD;
	uint32_t immediate;
	uint32_t address;
} buffer_EX;

//Structure for Execute and Memory Stage 

typedef struct EXE_MEM
{
int branch;
int memRead;
int memWrite;
int RegDst;
int RegDesNumber;
uint32_t result;
uint32_t RT;           // USED IN DATA MEM SECTION for read or write
uint32_t RS;
uint32_t RD;
int RegWrite;			 
} buffer_mem;

//Structure for Memory and Write Back Stage
typedef struct MEM_WB
{
int RegWrite;
int MemtoReg;
int RegDst;
int RegDesNumber;
uint32_t result;
uint32_t RT;           // USED IN DATA MEM SECTION for read or write
uint32_t RS;
uint32_t RD;
} buffer_wb;

//Extern variables for Buffer Registers to be used in pipe.c





/* global variable -- pipeline state */
extern CPU_State NEXT_STATE, CURRENT_STATE;

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
extern IF buffer_IF_ID;
extern IF emptyIFID;
void pipe_stage_decode();
extern buffer_EX buffer_ID_EX;
extern buffer_EX emptyIDEX;
void pipe_stage_execute();
extern buffer_mem buffer_EX_MEM;
extern buffer_mem emptyEXMEM;
void pipe_stage_mem();
extern buffer_wb buffer_MEM_WB;
extern buffer_wb emptyMEMWB;
void pipe_stage_wb();
void pipe_stage_ex_hazard();
void pipe_stage_mem_hazard();

#endif
