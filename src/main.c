#include <stdio.h>
#include "interaction_combinators.h"
#include <stdlib.h>




#define ERA_TAG 0
#define CON_TAG 1
#define STACK_SIZE 1000


struct era_node {
	int tag;
	void* main;
};

struct con_node {
	int tag;
	void* main;
	void* left;
	void* right;
	int label;
};


void push(void* value, void** stack, int* stack_top){
	stack[*stack_top] = value;
	(*stack_top)++;
}

struct era_node* new_era_node(void){
	struct era_node* node = malloc(sizeof(struct era_node));
	
	node->tag = ERA_TAG;
	return node;
}


void print_node(struct era_node* node){
	if (node->tag == ERA_TAG){
		printf("<Era node %p>\n", (void*)node);
	}
	else if (node->tag == CON_TAG){
		printf("<Combine node %p>\n", (void*)node);
	}
}


int reduce_era_con(struct era_node* a, struct con_node* b){

}
	

int reduce(void** stack, int* stack_top){
	(*stack_top)--;
	struct era_node* a = stack[*stack_top];
	struct era_node* b = a->main;
	print_node(a);
	print_node(b);

	if (a->tag == ERA_TAG){
		if (b->tag == ERA_TAG){
			printf("Reducing era-era\n");
			free(a);
			free(b);
			return 0;
		}
		else if (b->tag == CON_TAG){
			printf("Reducing era-con\n");
			return reduce_era_con(a, b);
		}
	}
	return 1;
}


int main(void) {
	void** stack = malloc(sizeof(void*) * STACK_SIZE);
	int stack_top = 0;

	for (int i = 0; i < 1; i++){
		struct era_node* a = new_era_node();
		struct era_node* b = new_era_node();
		a->main = b;
		b->main = a;
		push(a, stack, &stack_top);
		reduce(stack, &stack_top);
		printf()
	}

	free(stack);
	printf("Stack freed\n");
	return 0;
}
