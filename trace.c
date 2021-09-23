#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "loader.h"
#include "LC4.h"

int main(int argc, char* argv[]) {

	FILE *in, *out;

	for(int i = 2; i < argc; i++){
		in = fopen(argv[i], "r");
		loader(in);
	}
	out = fopen(argv[1], "w");

	decrypt(out, loader(in));

	return 0;
}
