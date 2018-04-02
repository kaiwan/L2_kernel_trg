/*
 *  mem_sequence.c / ver 0x01 | (C)opyleft 2008 by oozie |  http://blog.ooz.ie/
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Lists memory areas and their location in descending order.
 *
 *  This program declares example variables in different memory locations, gets
 *  their addresses, sorts them and prints a list in descending order. 
 *
 *  The program is considered free software; you can redistribute and/or modify
 *  it under the terms of the GNU General Public License version 3 or any later
 *  version, as published by the Free Software Foundation; If you do modify it, 
 *  please leave the  information about the author unchanged.  The full text of 
 *  latest GPL licence is available at      http://www.gnu.org/licences/gpl.txt
 *
 *  Src URL:
 *  http://oozie.fm.interia.pl/src/mem_sequence.c  
 */

#include <stdio.h>
#include <stdlib.h>

#define MEM_TYPES 5

#define STR_STACK "stack"
#define STR_HEAP  "heap"
#define STR_BSS   "bss"
#define STR_CONST "const's"
#define STR_CODE  "code"

const char const_example[] = "constant string";

/* 
 * defining a 5-element memory structure consisting of a string describing
 * memory type {stack, heap, bss, consts, code} and a pointer to an example
 * definition. Each element of this array is 12 bytes large (char[8]+*int),
 * so the entire table is 60 bytes in size.
 */

struct memtype_ptr {
	int *ptr;
	char *str;
};

struct memtype_ptr type_pointer[MEM_TYPES];

/* defining a global variable with predefined value (BSS) */
char bss_example[] = "A global variable";

/* mem_xchg() will be used later in print_sorted() function */
void mem_xchg(struct memtype_ptr *a, struct memtype_ptr *b)
{
	struct memtype_ptr tmpmemptr;

	tmpmemptr.str = a->str;
	tmpmemptr.ptr = a->ptr;

	a->str = b->str;
	a->ptr = b->ptr;

	b->str = tmpmemptr.str;
	b->ptr = tmpmemptr.ptr;

}

/* void function print_sorted() contains a local variable "stack_example" */
void print_sorted(int *heap_example, int stack_example)
{

	int i, j;

	/*
	 * assigning all example variables to the memory type pointer table 
	 * in no particular order 
	 */

	type_pointer[0].str = STR_HEAP;
	type_pointer[0].ptr = heap_example;

	type_pointer[1].str = STR_BSS;
	type_pointer[1].ptr = (int *)&bss_example;

	/*
	 * the address of stack_example, which is a local variable
	 * points us to the top of the stack
	 */

	type_pointer[2].str = STR_STACK;
	type_pointer[2].ptr = &stack_example;

	/*
	 * the value of stack_example equals to the value of code_example 
	 */

	type_pointer[3].str = STR_CODE;
	type_pointer[3].ptr = (int *)stack_example;

	type_pointer[4].str = STR_CONST;
	type_pointer[4].ptr = (int *)&const_example;

	/* buble-sorting the table in descending order */
	j = MEM_TYPES;
	while (j--)
		for (i = 0; i < j; i++)
			if (type_pointer[i].ptr < type_pointer[i + 1].ptr)
				mem_xchg(&type_pointer[i],
					 &type_pointer[i + 1]);

	/* printing the table */
	for (i = 0; i < MEM_TYPES; i++)
		printf("%d.(0x%.8x) %s\n", i + 1,
		       (int)type_pointer[i].ptr, type_pointer[i].str);

	return;
}

/* main */
int main(void)
{

	int *dyn_allocated, code_example;

	/* getting the address of main() in memory */
	code_example = (int)&main;

	/* allocating dynamic memory on the heap */
	dyn_allocated = (int *)malloc(sizeof(int));

	/*  printing memory addresses in descending order */
	print_sorted(dyn_allocated, code_example);
	getchar();

	return 0;
}
