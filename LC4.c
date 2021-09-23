#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "loader.h"
#include "LC4.h"

static unsigned short PC = 0x8200;
static unsigned short PSR = 0x8002;
static short registers[8];
static unsigned short regFileWE = 0;
static unsigned short NZPWE = 0;
static unsigned short DATAWE = 0;
static unsigned short address = 0;
static unsigned short memVal = 0;
static int NZP = 2;

void decrypt(FILE *out, unsigned short *memory) {
	unsigned short count = 1;
	while (PC != 0x80FF) {
		unsigned short opcode = (unsigned short) memory[PC] >> 12;
		unsigned short value = (unsigned short) memory[PC];
		printf("-----LINE NUMBER: %d -----\n", count);
		printf("OP: %d\n", opcode);
		printf("Val: %d\n", value);
		printf("PC: %d\n", PC);
		printf("NZP: %d\n", NZP);
		printf("PSR: %d\n", PSR);
		count++;
			if(opcode == 0) {
				br(out, value);
			}
			if(opcode == 1) {
				operations(out, value);
			}
			if(opcode == 2) {
				cmp(out, value);
			}
			if(opcode == 4) {
				jsr(out, value);
			}
			if(opcode == 5) {
				logic_gates(out, value);
			}
			if(opcode == 6) {
				ldr(out, value, memory);
			}
			if(opcode == 7) {
				str(out, value, memory);
			}
			if(opcode == 8) {
				rti(out, value);
			}
			if(opcode == 9) {
				constant(out, value);
			}
			if(opcode == 10) {
				shifter(out, value);
			}
			if(opcode == 12) {
				jmp(out, value);
			}
			if(opcode == 13) {
				hiconst(out, value);
			}
			if(opcode == 15) {
				trap(out, value);
			}
	}
}

void in_bounds(short imm, unsigned short rs) {
	if(PSR == 0 && (registers[rs] + imm >= 0xA000 && registers[rs] + imm <= 0xFFF)){
		exit(1);
	}
	if((registers[rs] + imm >= 0x0000 && registers[rs] + imm <= 0x1FFF) || (registers[rs] + imm >= 0x8000 && registers[rs] + imm <= 0x9FFF)){
		exit(1);
	}
}

int NZPtest(unsigned short regVal){
	if(((regVal >> 15) & 0x0001) == 1) {	
		return NZP = 4;
	} else if(regVal == 0x0000) {
		return NZP = 2;
	} else {
		return NZP = 1;
	}
}

void print_values(FILE *file, unsigned short value, unsigned short NZP, unsigned short reg, unsigned short regVal) {
	printPC(file);
	hex_to_binary(file, value);
	printRFWE(file);
	printReg(file, reg);
	printRegVal(file, regVal);
	printNZPWE(file);
	printNZP(file, NZP);
	printDATAWE(file);
	printAddress(file);
	printMemVal(file);
}

void br(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(value << 4) >> 13;
	short imm = (short)(value << 7 ) >> 7;

	address = 0;
	memVal = 0;
	regFileWE = 0;
	NZPWE = 0;
	DATAWE = 0;
	unsigned short regVal = 0;
	unsigned short reg = 0;

	if(instruction == 0) {
		print_values(file, value, 0, reg, regVal);
		PC = PC + 1;
	} else if((instruction == 1 && NZP == 1) || (instruction == 2 && NZP == 2) || (instruction == 3 && (NZP == 1 || NZP == 2)) ||
		(instruction == 4 && NZP == 4) || (instruction == 5 && (NZP == 4 || NZP == 1)) || 
		(instruction == 6 && (NZP == 4 || NZP == 2)) || instruction == 7) {
		imm = value;
		imm = imm & 0x1FF;
		if(imm & 0x100) {
			imm = imm | 0xFE00;
		}
		print_values(file, value, 0, reg, regVal);
		PC = PC + 1 + imm;
	} else {
		print_values(file, value, 0, reg, regVal);
		PC = PC + 1;
	}
}

void jsr(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(value << 4) >> 15;

	unsigned short rs = (unsigned short)(value << 7) >> 13;
	short imm = (short)(value << 5) >> 5;

	address = 0;
	memVal = 0;
	regFileWE = 1;
	DATAWE = 0;
	NZPWE = 1;
	unsigned short reg;
	unsigned short regVal;

	if(instruction == 0) {
		reg = 7;
		printPC(file);
		regVal = PC + 1;
		PC = registers[rs];
		NZP = NZPtest(regVal);
		registers[7] = regVal;
		hex_to_binary(file, value);
		printRFWE(file);
		printReg(file, reg);
		printRegVal(file, regVal);
		printNZPWE(file);
		printNZP(file, NZP);
		printDATAWE(file);
		printAddress(file);
		printMemVal(file);

	} else if(instruction == 1) {
		reg = 7;
		regVal = PC + 1;
		registers[7] = regVal;
		NZP = NZPtest(regVal);
		imm = value;
		imm = imm & 0x7FF;
		if(imm & 0x400) {
			imm = imm | 0xF800;
		}	
		print_values(file, value, NZP, reg, regVal);
		PC = (PC & 0x8000) | (imm << 4);
	}
}

void jmp(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(value << 4) >> 15;

	address = 0;
	memVal = 0;
	regFileWE = 0;
	NZPWE = 0;
	NZP = 0;
	DATAWE = 0;
	unsigned short regVal = 0;
	unsigned short reg = 0;

	if(instruction == 0) {
		unsigned short rs = (unsigned short)(value << 7) >> 13;
		print_values(file, value, NZP, reg, regVal);
		PC = registers[rs];
	} else if(instruction == 1) {
		short imm = value;
		imm = imm & 0x7FF;
		if(imm & 0x400){
			imm = imm | 0xF800;
		}
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1 + imm;
	} 
}

void constant(FILE *file, unsigned short value) {
	unsigned short rd = (unsigned short)(value << 4) >> 13;

	address = 0;
	memVal = 0;
	regFileWE = 1;
	NZPWE = 1;
	DATAWE = 0;
	short imm = value;
	imm = imm & 0x1FF;
	if(imm & 0x100) {
		imm = imm | 0xFE00;
	}
	unsigned short reg = rd;
	short regVal = imm;
	NZP = NZPtest(regVal);
	registers[rd] = imm; 
	print_values(file, value, NZP, reg, regVal);
	PC = PC + 1;

}

void hiconst(FILE *file, unsigned short value) {
	unsigned short rd = (unsigned short)(value << 4) >> 13;
	unsigned short uimm = (unsigned short)(value << 8) >> 8;
	
	address = 0;
	memVal = 0;
	regFileWE = 1;
	NZPWE = 1;
	DATAWE = 0;
	unsigned short reg = rd;
	unsigned short regVal = (registers[rd] & 0xFF) | (uimm << 8);
	NZP = NZPtest(regVal);
	registers[rd] = regVal;
	print_values(file, value, NZP, reg, regVal);
	PC = PC + 1;

}

void operations(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(((value) >> 12) & 15);
	unsigned short subinstruction = (unsigned short)(((value) >> 5) & 1);
	unsigned short sub = (unsigned short)(((value) >> 3) & 7);

	unsigned short rd = (unsigned short)(((value) >> 9) & 7);
	unsigned short rs = (unsigned short)(((value) >> 6) & 7);
	unsigned short rt = (unsigned short)((value) & 7);

	address = 0;
	memVal = 0;
	regFileWE = 1;
	NZPWE = 1;
	DATAWE = 0;
	unsigned short reg;
	short regVal;

	if(instruction == 1 && subinstruction == 1) {
		short imm = (short)(value << 11) >> 11;
		imm = imm & 0x1F;
		if(imm & 0x10) {
			imm = imm | 0xFFE0;
		}
		
		reg = rd;
		regVal = registers[rs] + imm;
		NZP = NZPtest(regVal);
		registers[reg] = (short)regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	} else if(instruction == 1 && sub == 0) {
		reg = rd;
		regVal = registers[rs] + registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 1 && sub == 1) {
		reg = rd;
		regVal = registers[rs] * registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	} else if(instruction == 1 && sub == 2) {
		reg = rd;
		regVal = registers[rs] - registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 1 && sub == 3) {
		reg = rd;
		regVal = registers[rs] / registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	}  
}

void shifter(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(value << 10) >> 14;

	unsigned short rd = (unsigned short)(value << 4) >> 13;
	unsigned short rs = (unsigned short)(value << 7) >> 13;
	unsigned short rt = (unsigned short)(value << 13) >> 13;
	unsigned short uimm = (unsigned short)(value << 12) >> 12;

	address = 0;
	memVal = 0;
	regFileWE = 1;
	NZPWE = 1;
	DATAWE = 0;
	unsigned short reg;
	unsigned short regVal;


	if(instruction == 0) {
		reg = rd;
		regVal = registers[rs]<<uimm;
		NZP = NZPtest(regVal);
		registers[rd]=regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	} else if(instruction == 1) {
		reg = rd;
		short val = (short)registers[rs];
		val = (short)val >> uimm;
		NZP = NZPtest(val);
		registers[rd] = val;
		print_values(file, value, NZP, reg, val);
		PC = PC + 1;
	} else if(instruction == 2) {
		reg = rd;
		regVal = registers[rs] >> uimm;
		NZP = NZPtest(regVal);
		registers[rd] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 3) {
		reg = rd;
		regVal = registers[rs]%registers[rt];
		NZP = NZPtest(regVal);
		registers[rd] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	} 
}

void logic_gates(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(value << 10) >> 13;

	unsigned short rd = (unsigned short)(value << 4) >> 13;
	unsigned short rs = (unsigned short)(value << 7) >> 13;
	unsigned short rt = (unsigned short)(value << 13) >> 13;

	unsigned short and_imm = (unsigned short)(value << 10) >> 15;

	address = 0;
	memVal = 0;
	regFileWE = 1;
	NZPWE = 1;
	DATAWE = 0;
	unsigned short reg;
	unsigned short regVal;

	if(instruction == 0) {
		reg = rd;
		regVal = registers[rs]&registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 1) {
		reg = rd;
		regVal = ~registers[rs];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 2) {
		reg = rd;
		regVal = registers[rs]|registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;	
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 3) {
		reg = rd;
		regVal = registers[rs]^registers[rt];
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(and_imm == 1) {
		short imm = (short)(value << 11) >> 11;
		imm = value;
		imm = imm & 0x1F;
		if(imm & 0x10) {
			imm = imm | 0xFFE0;
		}
		reg = rd;
		regVal = registers[rs]&imm;
		NZP = NZPtest(regVal);
		registers[reg] = regVal;
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	}
}

void ldr(FILE *file, unsigned short value, unsigned short *memory) {
	unsigned short rs = (unsigned short)(((value) >> 6) & 7);
	unsigned short rd = (unsigned short)(((value) >> 9) & 7);

	regFileWE = 1;
	NZPWE = 1;
	unsigned short reg;
	unsigned short regVal;
		
	short imm = ((value) & 63);
	imm = (((imm) >> 5) & 1);
	if (imm == 1) {
    	imm = ((imm) | 0xFFC0);
  	}

	in_bounds(imm, rs);

	reg = rd;
	regVal = memory[rs + imm];
	NZP = NZPtest(regVal);
	registers[rd] = regVal;
	print_values(file, value, NZP, reg, regVal);
	PC = PC + 1;
}

void str(FILE *file, unsigned short value, unsigned short *memory) {
	unsigned short rt = (unsigned short)((value) & 7);
	unsigned short rs = (unsigned short)(((value) >> 6) & 7);
	unsigned short rd = (unsigned short)(((value) >> 9) & 7);

	regFileWE = 0;
	NZPWE = 0;
	DATAWE = 1;
	unsigned short regVal = 0;
	unsigned short reg = 0;

	short imm = ((value) & 63);
	imm = (((imm) >> 5) & 1);
	if (imm == 1) {
    	imm = ((imm) | 0xFFC0);
  	}

	in_bounds(imm, rs);
	memory[rs+imm] = registers[rd];	

	address = registers[rs] + imm + rt;
	memVal = registers[rd];
	print_values(file, value, 0, reg, regVal);
	PC = PC + 1;

}

void cmp(FILE *file, unsigned short value) {
	unsigned short instruction = (unsigned short)(value << 7) >> 14;

	unsigned short rs = (unsigned short)(value << 4) >> 13;
	unsigned short rt = (unsigned short)(value << 13) >> 13;

	short imm = (short)(value << 9) >> 9;

	address = 0;
	memVal = 0;
	regFileWE = 0;
	NZPWE = 1;
	DATAWE = 0;
	unsigned short regVal = 0;
	unsigned short reg = 0;
 
	if(instruction == 0) {
		NZP = NZPtest((short)registers[rs] - (short)registers[rt]);
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;

	} else if(instruction == 1) {
		NZP = NZPtest(registers[rs] - registers[rt]);
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	} else if(instruction == 2) {
		imm = imm & 0x7F;
		if(imm & 0x40){
			imm = imm | 0xFF80;
		}
		NZP = NZPtest((registers[rs] - imm));
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	} else if(instruction == 3) {
		NZP = NZPtest(registers[rs] - imm);
		print_values(file, value, NZP, reg, regVal);
		PC = PC + 1;
	}
}

void rti(FILE *file, unsigned short value) {
	address = 0;
	memVal = 0;
	regFileWE = 0;	
	NZPWE = 0;
	DATAWE = 0;
	unsigned short regVal = 0;
	unsigned short reg = 0;
	print_values(file, value, 0, reg, regVal);
	PC = registers[7];
	PSR = 0;
}

void trap(FILE *file, unsigned short value) {
	unsigned short uimm = (unsigned short)(value << 8) >> 8;

	address = 0;
	memVal = 0;
	regFileWE = 1;
	NZPWE = 1;
	DATAWE = 0;
	unsigned short reg = 7;
	unsigned short regVal = PC + 1;
	registers[7] = regVal;
	NZP = NZPtest(regVal);
	print_values(file, value, NZP, reg, regVal);
	PC = (0x8000 | uimm);
	PSR = 1;
}

void hex_to_binary(FILE *file, unsigned short value){
	for(int i = 15; i >= 0; i--){
		if(((value >> i) & 0x0001) == 1){
			fprintf(file, "%d", 1);
		} else {
			fprintf(file, "%d", 0);
		}
	}
	fprintf(file, " ");
}

void printPC(FILE *file){
	fprintf(file, "%04hX ", PC);
}

void printRFWE(FILE *file){
	fprintf(file, "%01hX ", regFileWE);
}

void printReg(FILE *file, unsigned short reg){
	fprintf(file, "%01hX ", reg);
}

void printRegVal(FILE *file, unsigned short regVal){
	fprintf(file, "%04hX ", regVal);
}

void printNZPWE(FILE *file){
	fprintf(file, "%01hX ", NZPWE);
}

void printNZP(FILE *file, int NZP){
	fprintf(file, "%01X ", NZP);
}

void printDATAWE(FILE *file){
	fprintf(file, "%01hX ", DATAWE);
}

void printAddress(FILE *file){
	fprintf(file, "%04X ", address);
}

void printMemVal(FILE *file){
	fprintf(file, "%04hX\n", memVal);
}
