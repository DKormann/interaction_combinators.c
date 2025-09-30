#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define STACK_SIZE 1000


/*
POLARIZED NODE TYPES
negative node: value
positive node: consumer
ERA:
	main: +

NULL:
	main: -

LAM:
	main: - (function)
	aux1: - (parameter)
	aux2: + (body)

APP:
	main: + (function)
	aux1: + (argument)
	aux2: - (result)

DUP:
  int label
	main: + (value)
	aux1: - (var1)
	aux2: - (var2)

SUP:
	int label
	main: - (result)
	aux1: + (var1)
	aux2: + (var2)

*/



#define ERA_TAG 0
#define NULL_TAG 1
#define LAM_TAG 2
#define APP_TAG 3
#define DUP_TAG 4
#define SUP_TAG 5

int node_count = 0;

struct port {
	void* location;
	char port_number;
};

struct node{
	char tag;
	struct port main;
	struct port aux1;
	struct port aux2;
};

void push(void* value, void** stack, int* stack_top){
	stack[*stack_top] = value;
	(*stack_top)++;
}


char get_tag(void* node){
	return ((struct node*)node)->tag;
}

char get_arity(char tag){
	switch (tag){
		case ERA_TAG:
		case NULL_TAG:
			return 1;
		default:
			return 3;
	}
}

char get_polarity(char tag, char port_number){
	switch (tag){
		case ERA_TAG:
			return 1;
		case NULL_TAG:
			return -1;
		case LAM_TAG:
			return port_number == 2 ? 1 : -1;
		case APP_TAG:
			return port_number == 1 ? -1 : 1;
		case DUP_TAG:
			return port_number == 0 ? 1 : -1;
		case SUP_TAG:
			return port_number == 0 ? -1 : 1;
		default:
			return 0;
	}
}

char* rep_node(struct node* node){
	switch (get_tag(node)){
		case ERA_TAG:
			return "<Era node %p>";
		case NULL_TAG:
			return "<Null node %p>";
		case LAM_TAG:
			return "<Lam node %p>";
		case APP_TAG:
			return "<App node %p>";
		case DUP_TAG:
			return "<Dup node %p>";
		case SUP_TAG:
			return "<Sup node %p>";
		default:
			return "<Unknown node %p>";
	}
	return "<Unknown node %p>";
}

struct node* new_node(char tag){
	node_count++;
	struct node* node = malloc(sizeof(struct node));
	node->tag = tag;
	return node;
}

void free_node(struct node* node){
	node_count--;
	free(node);
}

char arity(char tag){
	if (tag == ERA_TAG || tag == NULL_TAG){
		return 1;
	}
	return 3;
}

int set_port(struct node* node, char port_number, void* location, char target_port_number){

	if (arity(node->tag) == 1 && port_number > 0){
		printf("Error: Cannot set port %d to node %s\n", port_number, rep_node(node));
		return 1;
	};

	struct port* port = 0;

	switch (port_number){
		case 0:
			port = &node->main;
			break;
		case 1:
			port = &node->aux1;
			break;
		case 2:
			port = &node->aux2;
			break;
		default:
			printf("Error: Invalid port number %d\n", port_number);
			return 1;
	}

	port->location = location;
	port->port_number = target_port_number;

	return 0;
}

int connect_ports(struct node* a, char an, struct node* b, char bn, void** stack, int* stack_top){

	if (get_polarity(a->tag, an) == get_polarity(b->tag, bn)){
		printf("Error: Polarities do not match for ports %s.%d and %s.%d\n", rep_node(a), an, rep_node(b), bn);
		return 1;
	};

	if (set_port(a, an, b, bn)){
		return 1;
	};
	if (set_port(b, bn, a, an)){
		return 1;
	};

	if (an == 0 && bn == 0){
		push(a, stack, stack_top);
	}

	return 0;
}



int interact(struct node* a, struct node* b, void** stack, int* stack_top){

	switch (a->tag){
		case ERA_TAG:
			switch (b->tag){
				case NULL_TAG:{	
					free_node(a);
					free_node(b);
					return 0;
				}
				
				
				case LAM_TAG: {
					if (b->aux1.location == b->aux2.location){
						free_node(a);
						free_node(b);
						return 0;
					}
				}
				case SUP_TAG:{
					struct node *child1 = b->aux1.location;
					char child1_port = b->aux1.port_number;
					struct node *child2 = b->aux2.location;
					char child2_port = b->aux2.port_number;
					struct node* era1 = a;
					struct node* era2 = new_node(ERA_TAG);
					connect_ports(era1, 0, child1, child1_port, stack, stack_top);
					connect_ports(era2, 0, child2, child2_port, stack, stack_top);
					free_node(b);
					return 0;
				}
			}
		case APP_TAG:
			switch (b->tag){
				case NULL_TAG:
					printf("TODO: APP -> NULL\n");
					return 0;
				case LAM_TAG:
					printf("TODO: APP -> LAM\n");
					return 0;
				case SUP_TAG:
					printf("TODO: APP -> SUP\n");
					return 0;
			}
		case DUP_TAG:
			switch (b->tag){
				case NULL_TAG:
					printf("TODO: DUP -> NULL\n");
					return 0;
				case LAM_TAG:
					printf("TODO: DUP -> LAM\n");
					return 0;
				case SUP_TAG:
					printf("TODO: DUP -> SUP\n");
					return 0;
			}
		case NULL_TAG:
		case LAM_TAG:
		case SUP_TAG:
			switch (b->tag){
				case ERA_TAG:
				case APP_TAG:
				case DUP_TAG:
					return interact(b, a, stack, stack_top);
			}
	}
	printf("ERROR: Invalid interaction %s -> %s\n", rep_node(a), rep_node(b));
	return 1;
}

int reduce(void** stack, int* stack_top){

	(*stack_top)--;
	struct node* a = (struct node*)stack[*stack_top];
	struct node* b = a->main.location;

	return interact(a, b, stack, stack_top);
}



int main(void) {
	void** stack = malloc(sizeof(void*) * STACK_SIZE);
	int stack_top = 0;

	for (int i = 0; i < 1; i++){
		struct node* a = new_node(ERA_TAG);
		struct node* b = new_node(NULL_TAG);

		connect_ports(a, 0, b, 0, stack, &stack_top);
		push(a, stack, &stack_top);
		printf("Node count: %d\n", node_count);
		reduce(stack, &stack_top);
		printf("Node count: %d\n", node_count);
	}

	free(stack);

	printf("Stack freed\n");
	return 0;
}
