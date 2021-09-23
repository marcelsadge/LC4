#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void decrypt(FILE *out, unsigned short *memory);

void in_bounds(short imm, unsigned short rs);

void print_values(FILE *file, unsigned short value, unsigned short NZP, unsigned short reg, unsigned short regVal);

void br(FILE *file, unsigned short value);
void jsr(FILE *file, unsigned short value);
void jmp(FILE *file, unsigned short value);

void constant(FILE *file, unsigned short memory);
void hiconst(FILE *file, unsigned short value);

void operations(FILE *file, unsigned short memory);
void shifter(FILE *file, unsigned short value);
void logic_gates(FILE *file, unsigned short value);

void ldr(FILE *file, unsigned short value, unsigned short *memory);
void str(FILE *file, unsigned short value, unsigned short *memory);
void cmp(FILE *file, unsigned short value);

void rti(FILE *file, unsigned short value);
void trap(FILE *file, unsigned short value);

int NZPtest(unsigned short regVal);

void printPC(FILE *file);
void hex_to_binary(FILE *file, unsigned short value);
void printRFWE(FILE *file);
void printRegVal(FILE *file, unsigned short regVal);
void printReg(FILE *file, unsigned short reg);
void printNZPWE(FILE *file);
void printNZP(FILE *file, int NZP);
void printDATAWE(FILE *file);
void printAddress(FILE *file);
void printMemVal(FILE *file);
