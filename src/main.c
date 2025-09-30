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
	struct node* location;
	char port_number;
};

struct node{
	char tag;
	struct port main;
	struct port aux1;
	struct port aux2;
	char label;
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

int connect_nodes(struct node* a, struct node* b, char an, char bn, void** stack, int* stack_top){
	if (get_polarity(a->tag, an) == get_polarity(b->tag, bn)){
		printf("Error: Polarities do not match for ports %s.%d and %s.%d\n", rep_node(a), an, rep_node(b), bn);
		return 1;
	}
	if (set_port(a, an, b, bn)){
		return 1;
	}
	if (set_port(b, bn, a, an)){
		return 1;
	}
	if (an == 0 && bn == 0){
		push(a, stack, stack_top);
	}
	return 0;
}

int connect_ports(struct port a, struct port b, void** stack, int* stack_top){
	return connect_nodes(a.location, b.location, a.port_number, b.port_number, stack, stack_top);
}

int annihilate(struct node* a, struct node* b){
	free_node(a);
	free_node(b);
	return 0;
}

int erase(struct node* e, struct node* c, void** stack, int* stack_top){
	struct node* e2 = new_node(e->tag);
	connect_nodes(e, c->aux1.location, 0, c->aux1.port_number, stack, stack_top);
	connect_nodes(e2, c->aux2.location, 0, c->aux2.port_number, stack, stack_top);
	free_node(c);
	return 0;
}

int commute(struct node* a, struct node* b, void** stack, int* stack_top){
	struct node* a2 = new_node(a->tag);
	a2->label = a->label;
	struct node* b2 = new_node(b->tag);
	b2->label = b->label;
	connect_nodes(b->aux1.location, a, b->aux1.port_number, 0, stack, stack_top);
	connect_nodes(b->aux2.location, a2, b->aux2.port_number, 0, stack, stack_top);
	connect_nodes(a->aux1.location, b2, a->aux1.port_number, 0, stack, stack_top);
	connect_nodes(a->aux2.location, b, a->aux2.port_number, 0, stack, stack_top);
	connect_nodes(a, b, 2, 1, stack, stack_top);
	connect_nodes(a, b2, 1, 1, stack, stack_top);
	connect_nodes(a2, b, 2, 2, stack, stack_top);
	connect_nodes(a2, b2, 1, 2, stack, stack_top);
	return 0;
}

int cancel(struct node* a, struct node* b, void** stack, int* stack_top){
	connect_nodes(a->aux1.location, b->aux1.location, a->aux1.port_number, b->aux1.port_number, stack, stack_top);
	connect_nodes(a->aux2.location, b->aux2.location, a->aux2.port_number, b->aux2.port_number, stack, stack_top);
	free_node(a);
	free_node(b);
	return 0;
}

int interact(struct node* a, struct node* b, void** stack, int* stack_top){

	switch (a->tag){
		case ERA_TAG:
			switch (b->tag){
				case NULL_TAG:{
					return annihilate(a, b);
				}
				case LAM_TAG: {
					if (b->aux1.location == b->aux2.location){
						return annihilate(a,b);
					}
				}
				case SUP_TAG:{
					return erase(a, b, stack, stack_top);
				}
			}
		case APP_TAG:
			switch (b->tag){
				case NULL_TAG:
					return erase(b, a, stack, stack_top);
				case LAM_TAG:
					return cancel(a,b, stack, stack_top);
				case SUP_TAG:
					return commute(a,b, stack, stack_top);
			}
		case DUP_TAG:
			switch (b->tag){
				case NULL_TAG:
					return erase(b, a, stack, stack_top);
				case LAM_TAG:
					return commute(a,b, stack, stack_top);
				case SUP_TAG:
					if (a->label == b->label){
						return cancel(a,b, stack, stack_top);
					}
					return commute(a,b, stack, stack_top);
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



struct port era(void){
	struct node* node = new_node(ERA_TAG);
	return (struct port){
		.location = node,
		.port_number = 0
	};
}

struct port null(void){
	struct node* node = new_node(NULL_TAG);
	return (struct port){
		.location = node,
		.port_number = 0
	};
}

struct port bin(struct port a, struct port b, char tag){
	struct node* dup = new_node(tag);
	connect_nodes(dup, a.location, 1, a.port_number, NULL, NULL);
	connect_nodes(dup, b.location, 2, b.port_number, NULL, NULL);
	return (struct port){
		.location = dup,
		.port_number = 0
	};
}

struct port dup(struct port a, struct port b, char label){
	struct port r = bin(a, b, DUP_TAG);
	r.location->label = label;
	return r;
}

struct port sup(struct port a, struct port b, char label){
	struct port r = bin(a, b, SUP_TAG);
	r.location->label = label;
	return r;
}

int build_era_sup_null_null(void** stack, int* stack_top){
	struct port e = era();
	struct port a = null();
	struct port b = null();
	struct port d = sup(a, b, 0);
	connect_ports(e, d, stack, stack_top);
	return 0;
}

int build_dup0_era_era_sup0_null_null(void** stack, int* stack_top){
	struct port dee = dup(era(), era(), 0);
	struct port snn = sup(null(), null(), 0);
	connect_ports(dee, snn, stack, stack_top);
	return 0;
}

int build_dup0_era_era_sup1_null_null(void** stack, int* stack_top){
	connect_ports(dup(era(), era(), 0), sup(null(), null(), 1), stack, stack_top);
	return 0;
}

int execute_stack(void** stack, int* stack_top, int* interaction_count){
	while (*stack_top > 0){
		if (reduce(stack, stack_top)){
			return 1;
		}
		(*interaction_count)++;
	}
	return 0;
}

int run_test(int (*a)(void** stack, int* stack_top)){
	void ** stack = malloc(sizeof(void*) * STACK_SIZE);
	int stack_top = 0;
	int interaction_count = 0;

	a(stack, &stack_top);
	printf("Node count: %d\n", node_count);
	execute_stack(stack, &stack_top, &interaction_count);
	printf("Node count: %d\n", node_count);
	printf("Interaction count: %d\n", interaction_count);
	free(stack);
	return 0;
}

int main(void) {
	printf("Running test build_era_sup_null_null\n");
	run_test(build_era_sup_null_null);
	printf("Running test build_dup_era_era_sup_null_null\n");
	run_test(build_dup0_era_era_sup0_null_null);
	printf("Running test build_dup0_era_era_sup1_null_null\n");
	run_test(build_dup0_era_era_sup1_null_null);
	return 0;
}
