/*
 * MIPS pipeline timing simulator
 *
 Salvador Cartagena
 Santosh Pandey
 Tapan Tandon

 */

#include "pipe.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//Global Variables used in the program

int runbit_ctr=0;
int PCsrc=0;
int TempPC=0;
int branch=0;
int Btargetready=0;
int Fstall=0;
int Estall=0;
int FTstall=0;
int ETstall=0;
int temp_jump_cs=0;

int forwardA=0;
int forwardB=0;

CPU_State CURRENT_STATE;
CPU_State NEXT_STATE;

buffer_EX buffer_ID_EX;
buffer_EX emptyIDEX;

IF buffer_IF_ID;
IF emptyIFID;

buffer_mem buffer_EX_MEM;
buffer_mem emptyEXMEM;

buffer_wb buffer_MEM_WB;
buffer_wb emptyMEMWB;

//Function to Implement Sign Extension

uint32_t extension(uint32_t temp, int length)
{
		//Shift to get MSB
	int x = (temp>>(length-1));
	if(x==0)
		//Using and with 0000FFFF
		{
			return(temp & 0x0000FFFF);
		}
	else
		//Or it with FFFF0000
		{
			return (temp | 0xFFFF0000);
		}

}

//Function to Flush the Instructions

void flush()
{
			PCsrc=1;
			buffer_ID_EX.memRead=0;
			buffer_ID_EX.memWrite=0;
			buffer_ID_EX.ALUsrc=0;
			buffer_ID_EX.ALUCTRL=0;
			buffer_ID_EX.MemtoReg=0;
			buffer_ID_EX.RegWrite=0;
			buffer_ID_EX.RegDst=0;
			buffer_ID_EX.PCSrc=1;
			buffer_ID_EX.branch=0;
			buffer_ID_EX.shamt=0;
			buffer_ID_EX.opcode=0;
			buffer_ID_EX.funct=0;
			buffer_ID_EX.RS=0;
			buffer_ID_EX.RT=0;
			buffer_ID_EX.RD=0;
			buffer_ID_EX.immediate=0;
			buffer_ID_EX.address=0;
			buffer_IF_ID.instruction=0;

}

//Function setting the initial Current State Program Counter

void pipe_init()
{
    memset(&CURRENT_STATE, 0, sizeof(CPU_State));
    CURRENT_STATE.PC = 0x00400000;
}

//Function to set the Fetch and Execute cycle stall counters

void pipe_cycle()
{
   	if(Fstall) Fstall--;
    if(Estall) Estall--;
	pipe_stage_wb();
	pipe_stage_mem();
	pipe_stage_execute();
	pipe_stage_decode();
	pipe_stage_fetch();
    printf("------------------------------------------\n");
    hazard_forward();
    printf("\n\n----------------------Next Cycle------------------------------------------------\n\n");


     if(temp_jump_cs==1)
    {	
        PCsrc=1;
        temp_jump_cs=0;
    }

    if (branch==1)
	{
		PCsrc=1;
        buffer_IF_ID=emptyIFID;
        NEXT_STATE.PC-=4;
		Fstall=FTstall;
		Estall=ETstall;
		branch=0;
	}


	CURRENT_STATE=NEXT_STATE;

}

// Write Back stage 

void pipe_stage_wb()
{
    printf("----------WB------------\n");
    //printf("Regwrite status:%d\n",buffer_EX_MEM.RegWrite);
	if(buffer_MEM_WB.RegWrite==1)
	{
        printf("Write back\n");
        // Write to a register based upon RegDest
        if (buffer_MEM_WB.RegDst==1)
        {
            //
		    NEXT_STATE.REGS[buffer_MEM_WB.RegDesNumber] = buffer_MEM_WB.result;
            printf("wrote R: %x\n",NEXT_STATE.REGS[buffer_EX_MEM.RD]);
}

        else
        {
            // extend to immediate
		   // NEXT_STATE.REGS[buffer_MEM_WB.RT] = buffer_EX_MEM.RT;
            NEXT_STATE.REGS[buffer_MEM_WB.RegDesNumber] = buffer_MEM_WB.result;
            printf("wrote I: %x\n",NEXT_STATE.REGS[buffer_MEM_WB.RT]);
        }

    }
    else{printf("No Register write.\n");}
    //printf("Regwrite status:%d\n",buffer_EX_MEM.RegWrite);
}

//Memory Access Stage

void pipe_stage_mem()
{
    printf("----------Mem stage------------\n");
    //printf("Regwrite status:%d\n",buffer_EX_MEM.RegWrite);

	if(buffer_EX_MEM.memRead==1)
	{
		// read from memory : lw
		buffer_MEM_WB.result = mem_read_32(buffer_EX_MEM.result);
	}
	else if (buffer_EX_MEM.memWrite==1)
	{
		// Write to memory
		mem_write_32(buffer_EX_MEM.RegDesNumber,buffer_EX_MEM.result);
	}
	// forward everything to next state
	// Move to next cylce
    else
    {
        printf("No mem write\n");
    }
    //printf("Regwrite status:%d\n",buffer_MEM_WB.RegWrite);
    buffer_MEM_WB.RT= buffer_EX_MEM.RT;
    buffer_MEM_WB.RS= buffer_EX_MEM.RS;
    buffer_MEM_WB.RD= buffer_EX_MEM.RD;
    buffer_MEM_WB.RegWrite= buffer_EX_MEM.RegWrite;
    buffer_MEM_WB.RegDesNumber=buffer_EX_MEM.RegDesNumber;
    buffer_MEM_WB.result= buffer_EX_MEM.result;
    //printf("Regwrite status:%d\n",buffer_EX_MEM.RegWrite);

}

//Execute Cycle Stage

void pipe_stage_execute()
{
	printf("-----------Execute-----------\n");
    //printf("Regwrite status:%d\n",buffer_EX_MEM.RegWrite);
    printf("Opcode:%d \n",buffer_ID_EX.opcode);
	if(Estall){printf("Stalling execute\n");
		runbit_ctr=0;
	 buffer_EX_MEM=emptyEXMEM; return; }

	//Start of R-Type Instructions

	// R- Type Add Instruction

	if (buffer_ID_EX.funct==32 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("R Add\n");
		buffer_EX_MEM.result = buffer_ID_EX.reg1 + buffer_ID_EX.reg2;
	}

	// R- Type Addu Instruction

	else if (buffer_ID_EX.funct==33 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("R Addu\n");
		buffer_EX_MEM.result = buffer_ID_EX.reg1 + buffer_ID_EX.reg2;
	}

	// R- Type Sub Instruction

	else if (buffer_ID_EX.funct==34 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("R sub\n");
		buffer_EX_MEM.result = buffer_ID_EX.reg1 - buffer_ID_EX.reg2;
	}

	// R- Type Subu Instruction

	else if (buffer_ID_EX.funct==35 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("R subu\n");
		buffer_EX_MEM.result = buffer_ID_EX.reg1 - buffer_ID_EX.reg2;
	}

	// R- Type Slt Instruction

	else if (buffer_ID_EX.funct==42 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{

		printf("R slt\n");
		if(buffer_ID_EX.reg1  < buffer_ID_EX.reg2)
		{
			buffer_EX_MEM.result = 1;
		}

		else
		{
			buffer_EX_MEM.result = 0;
		}
        printf("Result for slt is:%x \n", buffer_EX_MEM.result);

	}


	// R- Type Sltu Instruction

	else if (buffer_ID_EX.funct==43 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("R sltu\n");
		if(buffer_ID_EX.reg1  < buffer_ID_EX.reg2)
		{
			buffer_EX_MEM.result = 1;
		}

		else
		{
			buffer_EX_MEM.result = 0;
		}

	}

	//Start of I-Type Instructions

	// I-Type Addi Instruction

	else if (buffer_ID_EX.ALUCTRL==2 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==1 && buffer_ID_EX.MemtoReg==0 && buffer_ID_EX.opcode==8)
	{
        printf("I Addi\n");
		uint32_t temp = extension(buffer_ID_EX.immediate, 16);

		buffer_EX_MEM.result = buffer_ID_EX.reg1 + temp;
        printf("The sum is %x\n",buffer_EX_MEM.result);

	}


	// I-Type Addiu Instruction

	else if (buffer_ID_EX.ALUCTRL==2 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==1 && buffer_ID_EX.MemtoReg==0 && buffer_ID_EX.opcode==9)
	{
        printf("I Addiu\n");
		uint32_t temp = extension(buffer_ID_EX.immediate, 16);

		buffer_EX_MEM.result = buffer_ID_EX.reg1 + temp;

	}


	// I-Type Lui Instruction

	else if (buffer_ID_EX.opcode==15 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==1 && buffer_ID_EX.MemtoReg==0)
	{
        printf("I Lui\n");
		buffer_EX_MEM.result = (buffer_ID_EX.immediate << 16) & 0xFFFF0000;

	}

	// I-Type Ori Instruction

	else if (buffer_ID_EX.opcode==13 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==1 && buffer_ID_EX.MemtoReg==0)
	{
        printf("I Ori\n");
		buffer_EX_MEM.result = buffer_ID_EX.reg1 | buffer_ID_EX.immediate;

	}

	// I-Type lw Instruction

	else if (buffer_ID_EX.ALUCTRL==2 && buffer_ID_EX.opcode==35 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==1 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==1 && buffer_ID_EX.MemtoReg==1)
	{
        printf("I Lw\n");
		buffer_ID_EX.immediate = extension(buffer_ID_EX.immediate,16);
		// Calculate the offset with immediate value
		buffer_EX_MEM.result= buffer_ID_EX.reg1 + buffer_ID_EX.immediate*4;

	}

	// I-Type sw Instruction

	else if (buffer_ID_EX.ALUCTRL==2 && buffer_ID_EX.opcode==43 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==1 && buffer_ID_EX.ALUsrc==1 && buffer_ID_EX.MemtoReg==1)
	{
        printf("I Sw\n");
		// Calculating the address to write
		buffer_ID_EX.immediate = extension(buffer_ID_EX.immediate,16);
		buffer_EX_MEM.result= buffer_ID_EX.reg1 + buffer_ID_EX.immediate*4;

	}   


	// I-Type beq Instruction

	else if (buffer_ID_EX.ALUCTRL==6 && buffer_ID_EX.opcode==4 && buffer_ID_EX.RegWrite==0 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
		if((buffer_ID_EX.reg1 - buffer_ID_EX.reg2)==0)
        {
         printf("I beq\n");
         uint32_t temp= extension(buffer_ID_EX.immediate,16);
         temp = temp <<2;
         TempPC =buffer_ID_EX.PC+ temp+4;
		 printf("The value in RS is: %u \n", buffer_ID_EX.reg1);
		 flush();
		 //buffer_ID_EX=emptyIDEX;
		 //buffer_IF_ID=emptyIFID;
		 }

	}

	// I-Type bne Instruction

	else if (buffer_ID_EX.ALUCTRL==6 && buffer_ID_EX.opcode==5 && buffer_ID_EX.RegWrite==0 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("I Bne\n");
		if((buffer_ID_EX.reg1 - buffer_ID_EX.reg2)!=0)
        {
         uint32_t temp= extension(buffer_ID_EX.immediate,16);
		 printf("Temp value after sign extension: %x \n",temp);
         temp = temp <<2;
		 printf("Temp value after shifting:%x \n",temp);
         TempPC =buffer_ID_EX.PC+ temp+4;
		 printf("The value in RS is: %u \n", buffer_ID_EX.RS);
		 printf("The branch address is %x \n", NEXT_STATE.PC);
		 printf("The PCSCRC is %d \n", PCsrc);
		 flush();
		 //buffer_ID_EX=emptyIDEX;
		 //buffer_IF_ID=emptyIFID;
		 } 
	}

	// I-Type bgtz Instruction

	else if (buffer_ID_EX.ALUCTRL==6 && buffer_ID_EX.opcode==7 && buffer_ID_EX.RegWrite==0 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
        printf("Bgtz\n");
		printf("The value in RS is: %u \n", buffer_ID_EX.reg1);
		if(buffer_ID_EX.reg1 > 0)
        {
            printf("Reg1: %x\n",buffer_ID_EX.reg1);
         uint32_t temp= extension(buffer_ID_EX.immediate,16);
         temp = temp <<2;
         TempPC =buffer_ID_EX.PC+ temp+4;
		 printf("Next state is: %x \n", NEXT_STATE.PC);
		 flush();
		 //buffer_ID_EX=emptyIDEX;
		 //buffer_IF_ID=emptyIFID;
		 }

	}


    // I- Type Slti Instruction

	else if (buffer_ID_EX.ALUCTRL==7 && buffer_ID_EX.RegWrite==1 && buffer_ID_EX.memRead==0 && buffer_ID_EX.memWrite==0 && buffer_ID_EX.ALUsrc==0 && buffer_ID_EX.MemtoReg==0)
	{
		printf("Slti\n");
        buffer_ID_EX.reg1 = extension(buffer_ID_EX.reg1,16);
        buffer_ID_EX.reg2 = extension(buffer_ID_EX.immediate,16);

        if(buffer_ID_EX.reg1  < buffer_ID_EX.immediate)
		{
			buffer_EX_MEM.result = 1;
		}

		else
		{
			buffer_EX_MEM.result = 0;
		}

	}

	// Transferrring control signals and register numbers from ID_EX buffer to EX_MEM buffer

                buffer_EX_MEM.RegDst=buffer_ID_EX.RegDst;
				buffer_EX_MEM.memRead=buffer_ID_EX.memRead;
				buffer_EX_MEM.RegWrite=buffer_ID_EX.RegWrite;
				buffer_EX_MEM.memWrite=buffer_ID_EX.memWrite;
                buffer_EX_MEM.branch=buffer_ID_EX.branch;
                buffer_EX_MEM.RS=buffer_ID_EX.RS;
                buffer_EX_MEM.RT=buffer_ID_EX.RT;
                buffer_EX_MEM.RD=buffer_ID_EX.RD;
                buffer_EX_MEM.RegDesNumber=buffer_ID_EX.RegDesNumber;
                
}


//Decode Stage Cycle

//Decode Function splits up the instruction to different components and differentiates between R,I and J Types.

void pipe_stage_decode()
{
	//Extracting Opcode
    printf("----------Decode------------\n");
    if (buffer_IF_ID.instruction==0)
    {
    printf("No Instruction\n");
    buffer_ID_EX=emptyIDEX;
	return;
    }
    if(Estall)
    {
    printf("Stalling\n");
	runbit_ctr=0;
    return;
    }

    buffer_ID_EX.opcode = buffer_IF_ID.instruction>>26;
	printf("Opcode:%u, Instruction:%x \n",buffer_ID_EX.opcode,buffer_IF_ID.instruction);
	buffer_ID_EX.PC=buffer_IF_ID.PC;

	//R-Type Instruction Decoding

	if(buffer_ID_EX.opcode ==0)
	{
        printf("R-type\n");
		buffer_ID_EX.RS= (buffer_IF_ID.instruction>>21) & 0x1F;
		buffer_ID_EX.reg1= CURRENT_STATE.REGS[buffer_ID_EX.RS];
		buffer_ID_EX.RT = (buffer_IF_ID.instruction>>16) & 0x1F;
		buffer_ID_EX.reg2= CURRENT_STATE.REGS[buffer_ID_EX.RT];
		buffer_ID_EX.RD = (buffer_IF_ID.instruction>>11) & 0x1F;
        buffer_ID_EX.RegDesNumber=buffer_ID_EX.RD;
		buffer_ID_EX.funct = buffer_IF_ID.instruction & 0x3F;
		buffer_ID_EX.shamt = (buffer_IF_ID.instruction>>6) & 0x1F;
        printf("Function value:%d \n",buffer_ID_EX.funct);
	}

	//J-Type Instruction Decoding

	else if (buffer_ID_EX.opcode==2 || buffer_ID_EX.opcode==3)
	{
        printf("J-type\n");
		buffer_ID_EX.address = buffer_IF_ID.instruction & 0x3FFFFFF;
		 // Getting the old PC
        uint32_t old=  CURRENT_STATE.PC;
        old = old & 0xF0000000;
        // Get the address and shift by 2 bits
        uint32_t address1= (buffer_ID_EX.address<<2);
        // Add both
        buffer_ID_EX.address= old | address1;
		printf("New Address: %x \n",buffer_ID_EX.address);
		TempPC=buffer_ID_EX.address;
        temp_jump_cs =1;
		runbit_ctr=0;

		//Flush the Instructions

		buffer_ID_EX=emptyIDEX;
        buffer_IF_ID=emptyIFID;

	}

	//I-Type Instruction Decoding

	else if(buffer_ID_EX.opcode==5 || buffer_ID_EX.opcode==4 || buffer_ID_EX.opcode==8 || buffer_ID_EX.opcode==9 || buffer_ID_EX.opcode==10 || buffer_ID_EX.opcode==13 || buffer_ID_EX.opcode==15 || buffer_ID_EX.opcode==35 || buffer_ID_EX.opcode==43 || buffer_ID_EX.opcode==7)
	{
        printf("I-type\n");
		buffer_ID_EX.RS= (buffer_IF_ID.instruction>>21) & 0x1F;
		buffer_ID_EX.reg1= CURRENT_STATE.REGS[buffer_ID_EX.RS];
		buffer_ID_EX.RT = (buffer_IF_ID.instruction>>16) & 0x1F;
        buffer_ID_EX.reg2= CURRENT_STATE.REGS[buffer_ID_EX.RT];
        buffer_ID_EX.RegDesNumber=buffer_ID_EX.RT;
		buffer_ID_EX.immediate = buffer_IF_ID.instruction & 0x000FFFF;

	}


	// Control Signals Transfer to clean up the pipeline in the previous buffer register

	//R-Type Control Signals Transfer
	
	if(buffer_ID_EX.opcode == 0)
	{
		buffer_ID_EX.ALUCTRL = 2;
		buffer_ID_EX.ALUsrc= 0;
		buffer_ID_EX.memRead = 0;
		buffer_ID_EX.RegWrite = 1;
		buffer_ID_EX.MemtoReg = 0;
		buffer_ID_EX.memWrite = 0;
        buffer_ID_EX.RegDst = 1;
		buffer_ID_EX.PCSrc = 0;
	}

	//I-Type Control Signals Transfer

	else if(buffer_ID_EX.opcode==4 || buffer_ID_EX.opcode==5 || buffer_ID_EX.opcode==8 || buffer_ID_EX.opcode==9 || buffer_ID_EX.opcode==10 || buffer_ID_EX.opcode==13 || buffer_ID_EX.opcode==15 || buffer_ID_EX.opcode==35 || buffer_ID_EX.opcode==43 || buffer_ID_EX.opcode==7)
	{

	//Switch Statement to differentiate I-Type Signals

		switch(buffer_ID_EX.opcode)
		{
			case 4:   //beq instruction
                printf("Beq\n");
				buffer_ID_EX.ALUCTRL= 6;
				buffer_ID_EX.ALUsrc= 0;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 0;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
                branch  = 1;
				buffer_ID_EX.PCSrc = 1;
                buffer_ID_EX.RegDst = 0;
                Fstall=2;
				break;

			case 5:   //bne instruction
                printf("Bne\n");
				buffer_ID_EX.ALUCTRL= 6;
				buffer_ID_EX.ALUsrc= 0;
				buffer_ID_EX.memRead= 0;
				buffer_ID_EX.RegWrite= 0;
				buffer_ID_EX.MemtoReg= 0;
				buffer_ID_EX.memWrite = 0;
                buffer_ID_EX.RegDst = 0;                
                branch  = 1;
				FTstall=2;                
				break;

			case 7:   //bgtz instruction
                printf("Bgtz\n");
				buffer_ID_EX.ALUCTRL = 6;
				buffer_ID_EX.ALUsrc = 0;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 0;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
                buffer_ID_EX.RegDst = 0;
                branch  = 1;
				buffer_ID_EX.PCSrc = 1;
                FTstall=2;           
				break;


			case 8:   //addi instruction
                printf("addi\n");
				buffer_ID_EX.ALUCTRL = 2;
				buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 1;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
				break;

			case 9:   //addiu instruction
                printf("addiu\n");
				buffer_ID_EX.ALUCTRL = 2;
				buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 1;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
				break;

			case 10:   //slti instruction
                printf("slti\n");
				buffer_ID_EX.ALUCTRL = 7;
				buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 1;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
				break;

			case 13:   //ori instruction
                printf("ori\n");
				buffer_ID_EX.ALUCTRL = 1;
				buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 1;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
				break;

			case 15:   //lui instruction
                printf("lui\n");
				buffer_ID_EX.ALUCTRL = 2;
				buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 1;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 0;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
				break;

			case 35:   //lw instruction
                printf("lw\n");
				buffer_ID_EX.ALUCTRL = 2;
				buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 1;
				buffer_ID_EX.RegWrite = 1;
				buffer_ID_EX.MemtoReg = 1;
				buffer_ID_EX.memWrite = 0;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
				break;

			case 43:   //sw instruction
                printf("sw\n");
				buffer_ID_EX.ALUCTRL = 2;
             	buffer_ID_EX.ALUsrc = 1;
				buffer_ID_EX.memRead = 0;
				buffer_ID_EX.RegWrite = 0;
				buffer_ID_EX.MemtoReg = 0;
				buffer_ID_EX.memWrite = 1;
				buffer_ID_EX.PCSrc = 0;
                buffer_ID_EX.RegDst = 0;
                break;
        }
    }

}


// Function to detect Ex Hazards

void hazard_forward()
{
    printf("EX MEM Regwrite: %x \n",buffer_EX_MEM.RegWrite);
    printf("EX MEM RegDest: %x \n",buffer_EX_MEM.RegDesNumber);
    printf("EX MEM RS: %x \n",buffer_EX_MEM.RS);
    printf("EX MEM RT: %x \n",buffer_EX_MEM.RT);
    printf("ID_EX RD: %x\n",buffer_ID_EX.RD);
    printf("ID_EX RS: %x\n",buffer_ID_EX.RS);    
    printf("ID_EX RT: %x\n",buffer_ID_EX.RT);

  if((buffer_EX_MEM.RegWrite) && (buffer_EX_MEM.RegDesNumber!=0) && (buffer_EX_MEM.RegDesNumber == buffer_ID_EX.RS))
    {
        printf("Ex Data hazard for RS and RD\n");
        forwardA=10;  

    }

     else if((buffer_EX_MEM.RegWrite) && (buffer_EX_MEM.RegDesNumber!=0) && (buffer_EX_MEM.RegDesNumber == buffer_ID_EX.RT))
    {
        printf("Ex Data hazard for RT and RD\n");
        forwardB=10;  

    }


// Function to detect Mem Hazards

  if((buffer_MEM_WB.RegWrite) && (buffer_MEM_WB.RegDesNumber!=0) && (buffer_MEM_WB.RegDesNumber == buffer_ID_EX.RS) && (!(((buffer_MEM_WB.RegWrite) && (buffer_MEM_WB.RegDesNumber!=0)) && (buffer_MEM_WB.RegDesNumber!=buffer_ID_EX.RS))))
    {
        printf("Data mem hazard for RS and RD\n");
        forwardA=1;  

    }

     if((buffer_MEM_WB.RegWrite) && (buffer_MEM_WB.RegDesNumber!=0) && (buffer_MEM_WB.RegDesNumber == buffer_ID_EX.RT))
    {
        printf("Data mem hazard for RT and RD\n");
        forwardB=1;  

    }


        // Hazard detection for load use word case

            if(buffer_ID_EX.memRead){
                printf("Mem read for next\n");
            if((buffer_EX_MEM.RT==buffer_ID_EX.RS)   || (buffer_EX_MEM.RT==buffer_ID_EX.RT) )
                {
                    // Stall the pipeline
                    printf("Stalling for Mem\n");
                    ETstall=1;    
					runbit_ctr=0;            
                }
               }

    if (forwardA== 10)
        {
        buffer_ID_EX.reg1= buffer_EX_MEM.result; 
        printf("Forwarding A: %x\n",buffer_ID_EX.reg1);
        forwardA=0;
        }
    if (forwardB== 10)
        {
        forwardB=0;
        buffer_ID_EX.reg2=buffer_EX_MEM.result;
        printf("Forwarding B: %x\n",buffer_ID_EX.reg2);        
        }    


    if (forwardA== 1)
    {
        forwardA=0;
        if(buffer_MEM_WB.MemtoReg){   
        buffer_ID_EX.reg1= buffer_MEM_WB.result; 
        printf("Forwarding A from mem%x\n",buffer_ID_EX.reg1);        
        } 
        
        else
            {buffer_ID_EX.reg1= buffer_EX_MEM.result;
            printf("Forwarding A from ex %x\n",buffer_ID_EX.reg2);                
            }
    }

     if (forwardB== 1)
    {
        forwardB=0;
        if(buffer_MEM_WB.MemtoReg){   
        buffer_ID_EX.reg2= buffer_MEM_WB.result; 
        printf("Forwarding B from mem %x\n",buffer_ID_EX.reg2);
        } 
        else
            {buffer_ID_EX.reg2= buffer_EX_MEM.result;
            printf("Forwarding B from ex %x\n",buffer_ID_EX.reg2);            
            }
    }

}

//Fetch Cycle Stage

void pipe_stage_fetch()
{
    printf("-----------Fetch-----------\n");
	printf("Current PC Address:%x\n",CURRENT_STATE.PC);
    //IF PC.src = 1, then do nothing, meaning dont add 4
	printf("PCSrc:%d\n",PCsrc);

	if(PCsrc == 0 && Fstall== 0)
	{
        buffer_IF_ID.instruction = mem_read_32(CURRENT_STATE.PC);
		buffer_IF_ID.PC=CURRENT_STATE.PC;
		printf("New Instruction:%x \n", buffer_IF_ID.instruction);
		NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	}

	else if (Fstall || Estall)
	{
		printf("Stalling Fetch\n");
		runbit_ctr=0;
	}

    
    else if(PCsrc==1)
	{
		printf("Jump/Branch Instruction executed \n");
		PCsrc=0;
        temp_jump_cs=0;
        if(TempPC)
        {
            printf("New address\n");
            buffer_IF_ID.instruction = mem_read_32(TempPC);
            buffer_IF_ID.PC=TempPC;
            TempPC=0;
        }
        else
		{buffer_IF_ID.instruction = mem_read_32(CURRENT_STATE.PC);
        buffer_IF_ID.PC=CURRENT_STATE.PC;		
        printf("Resuming Instruction:%x \n", buffer_IF_ID.instruction);}
                
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	}

    else{printf("I dnt know\n");}

	if(buffer_IF_ID.instruction==0)
	{
		runbit_ctr++;
		if(runbit_ctr==5)
		{
		printf("Simulation Halted \n");
		exit(0);
		}
	}

}

