#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define STACK_SIZE 1000


/*
POLARIZED NODE TYPES
negative Node: value
positive Node: consumer
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

typedef struct port {
	struct Node* location;
	char port_number;
}Port;





typedef struct Node{
	char tag;
	struct port main;
	struct port aux1;
	struct port aux2;
	char label;
} Node;

typedef struct Runtime{
	Node** stack;
	int stack_top;
} Runtime;

Runtime fresh_runtime(void){
	return (Runtime){.stack = malloc(sizeof(Node*) * STACK_SIZE), .stack_top = 0};
}

void free_runtime(Runtime* runtime){
	free(runtime->stack);
	// free(runtime);
}


// typedef struct Node Node;


void push(void* value, Runtime* runtime){
	runtime->stack[runtime->stack_top] = value;
	runtime->stack_top++;
}


char get_tag(void* Node){
	return ((struct Node*)Node)->tag;
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
			return port_number == 2 ? -1 : 1;
		case DUP_TAG:
			return port_number == 0 ? 1 : -1;
		case SUP_TAG:
			return port_number == 0 ? -1 : 1;
		default:
			return 0;
	}
}

char* rep_node(struct Node* Node){
	switch (get_tag(Node)){
		case ERA_TAG:
			return "<Era Node %p>";
		case NULL_TAG:
			return "<Null Node %p>";
		case LAM_TAG:
			return "<Lam Node %p>";
		case APP_TAG:
			return "<App Node %p>";
		case DUP_TAG:
			return "<Dup Node %p>";
		case SUP_TAG:
			return "<Sup Node %p>";
		default:
			return "<Unknown Node %p>";
	}
	return "<Unknown Node %p>";
}

struct Node* new_node(char tag){
	node_count++;
	struct Node* Node = malloc(sizeof(struct Node));
	Node->tag = tag;
	return Node;
}


void free_node(struct Node* Node){
	node_count--;
	free(Node);
}

char arity(char tag){
	if (tag == ERA_TAG || tag == NULL_TAG){
		return 1;
	}
	return 3;
}

int set_port(struct Node* Node, char port_number, void* location, char target_port_number){

	if (arity(Node->tag) == 1 && port_number > 0){
		printf("Error: Cannot set port %d to Node %s\n", port_number, rep_node(Node));
		return 1;
	};

	struct port* port = 0;

	switch (port_number){
		case 0:
			port = &Node->main;
			break;
		case 1:
			port = &Node->aux1;
			break;
		case 2:
			port = &Node->aux2;
			break;
		default:
			printf("Error: Invalid port number %d\n", port_number);
			return 1;
	}

	port->location = location;
	port->port_number = target_port_number;

	return 0;
}

int connect_nodes(struct Node* a, struct Node* b, char an, char bn, Runtime* runtime){
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
		push(a, runtime);
	}
	return 0;
}

int connect_ports(struct port a, struct port b, Runtime* runtime){
	return connect_nodes(a.location, b.location, a.port_number, b.port_number, runtime);
}

int add_aux(struct Node* Node, char port_number, struct port a){
	if (port_number==0){
		printf("Not an aux port\n");
		return 1;
	}
	return connect_nodes(Node, a.location, port_number, a.port_number, NULL);
}

int add_auxs(struct Node* Node, struct port a, struct port b){
	if (add_aux(Node, 1, a)){
		return 1;
	}
	return add_aux(Node, 2, b);
}


int annihilate(struct Node* a, struct Node* b){
	free_node(a);
	free_node(b);
	return 0;
}

int erase(struct Node* e, struct Node* c, Runtime* runtime){
	struct Node* e2 = new_node(e->tag);
	connect_nodes(e, c->aux1.location, 0, c->aux1.port_number, runtime);
	connect_nodes(e2, c->aux2.location, 0, c->aux2.port_number, runtime);
	free_node(c);
	return 0;
}

int commute(struct Node* a, struct Node* b, Runtime* runtime){
	struct Node* a2 = new_node(a->tag);
	a2->label = a->label;
	struct Node* b2 = new_node(b->tag);
	b2->label = b->label;
	connect_nodes(b->aux1.location, a, b->aux1.port_number, 0, runtime);
	connect_nodes(b->aux2.location, a2, b->aux2.port_number, 0, runtime);
	connect_nodes(a->aux1.location, b2, a->aux1.port_number, 0, runtime);
	connect_nodes(a->aux2.location, b, a->aux2.port_number, 0, runtime);
	connect_nodes(a, b, 2, 1, runtime);
	connect_nodes(a, b2, 1, 1, runtime);
	connect_nodes(a2, b, 2, 2, runtime);
	connect_nodes(a2, b2, 1, 2, runtime);
	return 0;
}

int cancel(struct Node* a, struct Node* b, Runtime* runtime){
	connect_nodes(a->aux1.location, b->aux1.location, a->aux1.port_number, b->aux1.port_number, runtime);
	connect_nodes(a->aux2.location, b->aux2.location, a->aux2.port_number, b->aux2.port_number, runtime);
	free_node(a);
	free_node(b);
	return 0;
}

typedef enum {
	NODE_TYPE_ERA,
	NODE_TYPE_NULL,
	NODE_TYPE_LAM,
	NODE_TYPE_APP,
	NODE_TYPE_DUP,
	NODE_TYPE_SUP,
} node_type;


Port* get_port(struct Node* Node, char port_number){
	switch (port_number){
		case 0:
			return &Node->main;
		case 1:
			return &Node->aux1;
		case 2:
			return &Node->aux2;
	}
	printf("Error: Invalid port number %d\n", port_number);
	return NULL;
}

int check_port(Node* location, char port_number){

	Node* node_a = location;
	Port* port_a = get_port(node_a, port_number);

	Node* node_b = port_a->location;
	Port* port_b = get_port(node_b, port_a->port_number);
	
	if (port_b->location != location){
		return 1;
	}

	if (port_b->port_number != port_number){
		return 1;
	}

	return 0;
}

int check_node(struct Node* Node){

	for (int i = 0; i < get_arity(Node->tag); i++){
		if (check_port(Node, i)){
			printf("Error: Invalid port %d for Node %s\n", i, rep_node(Node));
			return 1;
		}
	}
	return 0;

}

int interact(struct Node* a, struct Node* b, Runtime* runtime){

	if (check_node(a)){
		return 1;
	}
	if (check_node(b)){
		return 1;
	}

	switch (a->tag){
		case ERA_TAG:
			switch (b->tag){
				case NULL_TAG:return annihilate(a, b);
				case LAM_TAG:
					if (b->aux1.location == b->aux2.location){
						return annihilate(a,b);
					}
				case SUP_TAG: return erase(a, b, runtime);
			}
		case APP_TAG:
			switch (b->tag){
				case NULL_TAG: return erase(b, a, runtime);
				case LAM_TAG: return cancel(a,b, runtime);
				case SUP_TAG: return commute(a,b, runtime);
			}
		case DUP_TAG:
			switch (b->tag){
				case NULL_TAG: return erase(b, a, runtime);
				case SUP_TAG:
					if (a->label == b->label){
						return cancel(a,b, runtime);
					}
				case LAM_TAG: return commute(a,b, runtime);
			}
		case NULL_TAG:
		case LAM_TAG:
		case SUP_TAG:
			switch (b->tag){
				case ERA_TAG:case APP_TAG:case DUP_TAG:
					return interact(b, a, runtime);
			}
	}
	printf("ERROR: Invalid interaction %s -> %s\n", rep_node(a), rep_node(b));
	return 1;
}

int reduce(Runtime* runtime){

	runtime->stack_top--;
	struct Node* a = (struct Node*)runtime->stack[runtime->stack_top];
	struct Node* b = a->main.location;

	return interact(a, b, runtime);
}



struct port era(void){
	struct Node* Node = new_node(ERA_TAG);
	return (struct port){
		.location = Node,
		.port_number = 0
	};
}

struct port null(void){
	struct Node* Node = new_node(NULL_TAG);
	return (struct port){
		.location = Node,
		.port_number = 0
	};
}

Port mk_port(Node* Node, char port_number){
	return (Port){.location = Node, .port_number = port_number};
}

Port mk_main(Node* Node){
	return mk_port(Node, 0);
}

Port bin(Port a, Port b, char tag){
	Node* node = new_node(tag);
	add_auxs(node, a, b);
	return mk_main(node);
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

int execute_stack(Runtime* runtime, int* interaction_count){
	while (runtime->stack_top > 0){
		if (reduce(runtime)){
			return 1;
		}
		(*interaction_count)++;
	}
	return 0;
}

struct test_error{
	char* test_name;
	char* error_message;
	struct test_error* next;
};


struct test_error* test_errors = NULL;



Port mk_aux_1(Node* Node){
	return mk_port(Node, 1);
}

Port mk_aux_2(Node* Node){
	return mk_port(Node, 2);
}

Port var(Port lam){
	if (lam.location->tag != LAM_TAG){
		printf("Error: Node is not a lambda\n");
	}
	return (Port){.location = lam.location, .port_number = 1};
}

Port mk_lam(void){
	return mk_main(new_node(LAM_TAG));
}

int add_return(struct port a, struct port b){
	return add_aux(a.location, 2, b);
}

Port mk_app(Port fn, Port arg, Runtime* runtime){
	Node * app = new_node(APP_TAG);
	add_aux(app, 1, arg);
	connect_ports(fn, mk_main(app), runtime);
	return mk_aux_2(app);
}

Port mk_id(void){
	Port lam = mk_lam();
	add_return(lam, var(lam));
	return lam;
}

Port church_true(void){
	Port lam = mk_lam();
	Port bod = mk_lam();
	add_return(lam, bod);
	add_return(bod, var(lam));
	connect_ports(var(bod), era(), NULL);
	return lam;
}

Port church_false(void){
	Port lam = mk_lam();
	add_auxs(lam.location, era(), mk_id());
	return lam;
}

Port church_zero(void){
	return church_false();
}

Port var_dup(Port* port){
	Node* dup = new_node(DUP_TAG);
	dup->main.location = port->location;
	dup->main.port_number = port->port_number;
	port->location = dup;
	port->port_number = 1;
	return (Port){.location = dup, .port_number = 2};
}

Port church_one(Runtime* runtime){
	Port flam = mk_lam();
	Port xlam = mk_lam();
	add_return(flam, xlam);
	Port f = var(flam);
	Port x = var(xlam);
	add_return(xlam, mk_app(f, x, runtime));
	return flam;
}

Port church_two(Runtime* runtime){
	Port flam = mk_lam();
	Port xlam = mk_lam();
	Port f = var(flam);
	Port f2 = var_dup(&f);
	add_return(flam, xlam);
	Port inner_app = mk_app(f2, var(xlam), runtime);
	add_return(xlam, mk_app(f, inner_app, runtime));
	return flam;
}




int lam_depth(Node* node){
	if (node->main.location == NULL){
		return 0;
	}
	switch (node->main.location->tag){
		case LAM_TAG:
			return 1 + lam_depth(node->main.location);
		case APP_TAG:
			return lam_depth(node->aux2.location);
	}
	printf("Error: cant find lam depth in %s\n", rep_node(node));
	return 10000;
}


int do_print_term(Port port, int depth){

	if (get_polarity(port.location->tag, port.port_number) == 1){
		printf("ERROR: positive port is not a term: %s.%d\n", rep_node(port.location), port.port_number);
	}
	
	Node* node = port.location;
	switch (node->tag){
		case LAM_TAG:
			if (port.port_number == 1){
				int myd = lam_depth(node);
				printf("x%d ", depth - myd - 1);
				return 0;
			}
			printf("λ ");
			do_print_term(node->aux2, depth + 1);
			return 0;
		case APP_TAG:
			printf("(");
			do_print_term(node->main, depth );
			do_print_term(node->aux1, depth );
			printf(") ");
			return 0;
		case DUP_TAG:
			do_print_term(node->main, depth );
			return 0;
		case NULL_TAG:
			printf("NULL ");
			return 0;
	}
	printf("Error: cant print node %s\n", rep_node(node));
	return 1;
}

int print_node(Port port){
	do_print_term(port, 0);
	printf("\n");
	return 0;
}


int main(void) {
	Runtime runtime = fresh_runtime();	
	print_node(null());
	print_node(mk_id());


	print_node(church_false());
	print_node(church_true());
	print_node(church_one(&runtime));
	print_node(church_two(&runtime));
	free_runtime(&runtime);	


	return 0;
}
