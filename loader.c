#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "loader.h"
#include "LC4.h"

unsigned short *loader(FILE* file) {
	static unsigned short memory[65536];

	unsigned short int sizeofShort = sizeof(unsigned short);
	unsigned short val, instruction;
	unsigned short address = 0;
	unsigned short label = 0;
	unsigned short line = 0;
	unsigned short findex = 0;
	unsigned short inst_val = 0;
	unsigned short chars = 0;
	unsigned short i = 0;

	while(fread(&val, sizeofShort, 1, file) > 0) {
		if((val == (unsigned short) 0xDECA) || (val == (unsigned short) 0xDADA)) {
			fread(&val, sizeofShort, 1, file);
			address = (val << 8) | (val >> 8);
			fread(&val, sizeofShort, 1, file);
			inst_val = (val << 8) | (val >> 8);

			for(i = 0; i < inst_val; i++) {
				fread(&val, sizeofShort, 1, file);
				instruction = (val << 8) | (val >> 8);
				memory[address] = instruction;
				address++;
			}
		}
		if(val == (unsigned short) 0xB7C3) {
			fread(&val, sizeofShort, 1, file);
			address = (val << 8) | (val >> 8);
			fread(&val, sizeofShort, 1, file);
			chars = (val << 8) | (val >> 8);

			for(i = 0; i < chars; i++) {
				fread(&val, 1, 1, file);
				address++;
			}
		}
		if(val == (unsigned short) 0x7EF1) {
			fread(&val, sizeofShort, 1, file);
			chars = (val << 8) | (val >> 8);

			for(i = 0; i < chars; i++) {
				fread(&val, sizeofShort, 1, file);
				label = ((val & 0x0F)<<4 | (val & 0xF0)>>4);
				address++;
			}
		}
		if(val == (unsigned short) 0x5E71) {
			fread(&val, sizeofShort, 1, file);
			address = (val << 8) | (val >> 8);
			fread(&val, sizeofShort, 1, file);
			line = (val << 8) | (val >> 8);
			fread(&val, sizeofShort, 1, file);
			findex = (val << 8) | (val >> 8);

		}
	}	
	return &memory[0];
}
