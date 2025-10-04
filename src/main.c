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



#define NULL_TAG 1
#define LAM_TAG 2
#define APP_TAG 3
#define DUP_TAG 4
#define SUP_TAG 5
#define ERA_TAG 6
#define ROOT_TAG 7

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
		case ROOT_TAG:
			return 1;
		default:
			return 3;
		
	}
}

char get_polarity(char tag, char port_number){
	switch (tag){
		case ERA_TAG:
		case ROOT_TAG:
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

char* tag_name(Node* node){
	if (node == NULL){
		return "ERR";
	}
	switch (node->tag){
		case ERA_TAG:
			return "Era";
		case NULL_TAG:
			return "Nul";
		case LAM_TAG:
			return "Lam";
		case APP_TAG:
			return "App";
		case DUP_TAG:
			return "Dup";
		case SUP_TAG:
			return "Sup";
		case ROOT_TAG:
			return "Roo";
	}
	printf("Error: Invalid tag %d\n", node->tag);
	return "UNK";
}

char* format_node(struct Node* Node){
	if (Node == NULL){
		return "NULL";
	}
	char* buf = malloc(30);
	if (get_arity(Node->tag) == 1){
		sprintf(buf, "<%s: %s>", tag_name(Node), tag_name(Node->main.location));
	}else{
		sprintf(buf, "<%s: %s %s %s>", tag_name(Node), tag_name(Node->main.location), tag_name(Node->aux1.location), tag_name(Node->aux2.location));
	}
	return buf;
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



void free_term(Port port){
	Node* node = port.location;
	switch (node->tag){
		case LAM_TAG:
			if (port.port_number == 1){
				return;
			}
			free_term(node->aux2);
			break;
		case APP_TAG:
			free_term(node->main);
			free_term(node->aux1);
			break;
		case DUP_TAG:
			get_port(node, port.port_number)->location = NULL;
			char other_port_number = 3 - port.port_number;
			if (get_port(node, other_port_number)->location == NULL){
				break;
			}else{
				return;
			}	
	}
	free_node(node);
	return;

}

char arity(char tag){
	if (tag == ERA_TAG || tag == NULL_TAG){
		return 1;
	}
	return 3;
}

int set_port(struct Node* Node, char port_number, void* location, char target_port_number){

	if (arity(Node->tag) == 1 && port_number > 0){
		printf("Error: Cannot set port %d to Node %s\n", port_number, format_node(Node));
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
		printf("Error: Polarities do not match for ports %s.%d and %s.%d\n", format_node(a), an, format_node(b), bn);
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




typedef enum {
	NODE_TYPE_ERA,
	NODE_TYPE_NULL,
	NODE_TYPE_LAM,
	NODE_TYPE_APP,
	NODE_TYPE_DUP,
	NODE_TYPE_SUP,
} node_type;



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
			printf("Error: Invalid port %d for Node %s\n", i, format_node(Node));
			return 1;
		}
	}
	return 0;

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

struct port root(void){
	struct Node* Node = new_node(ROOT_TAG);
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

int lam_depth(Node* node){
	if (node == NULL){
		return 0;
	}

	switch (node->tag){
		case LAM_TAG:
			if (node->main.location == NULL){
				return 0;
			}
			return 1 + lam_depth(node->main.location);
		case APP_TAG:
			if (node->aux2.location == NULL){
				return 0;
			}
			return lam_depth(node->aux2.location);
		case ROOT_TAG:
			return 0;
	}
	printf("Error: cant find lam depth in %s\n", format_node(node));
	return 10000;
}


char* string_malloc(char* a){
	char* result = malloc(strlen(a) + 1);
	strcpy(result, a);
	return result;
}

char* string_concat(char* a, char* b){
	int len_a = strlen(a);
	int len_b = strlen(b);
	char* result = malloc(len_a + len_b + 1);
	strcpy(result, a);
	strcpy(result + len_a, b);
	free(a);
	free(b);
	return result;
}



char* do_format_term(Port port, int depth){
	switch (port.location->tag){
		case LAM_TAG:
			if (port.port_number == 1){
				int myd = lam_depth(port.location);
				{
					char buf[32];
					snprintf(buf, sizeof(buf), "x%d", depth - myd);
					return string_malloc(buf);
				}
			}
			char* aux2 = do_format_term(port.location->aux2, depth + 1);
			return string_concat(string_malloc("λ "),  aux2);
		case APP_TAG:{
				printf("app tag\n");
				char* sa = do_format_term(port.location->main, depth);
				char* sb = do_format_term(port.location->aux1, depth);
				char* buf = malloc(strlen(sa) + strlen(sb) + 3);
				sprintf(buf, "(%s %s)", sa, sb);
				free(sa);
				free(sb);
				return buf;
			}
		case DUP_TAG:
			printf("dup tag\n");
		  return do_format_term(port.location->main, depth);
		case NULL_TAG:
			printf("null tag\n");
			return string_malloc("NULL");
	}
	// return string_malloc("UNKOWN TERM");
	// return 
	char*errmsg = "UNKOWN TERM %s";
	char* buf = malloc(strlen(errmsg) + 40);
	sprintf(buf, "UNKOWN TERM %s", tag_name(port.location));
	return buf;
}


char* format_term(Port port){
	return do_format_term(port, 0);
}



/* INTERACTIONS */


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

int anihilate(struct Node* a, struct Node* b, Runtime* runtime){
	connect_nodes(a->aux1.location, b->aux1.location, a->aux1.port_number, b->aux1.port_number, runtime);
	connect_nodes(a->aux2.location, b->aux2.location, a->aux2.port_number, b->aux2.port_number, runtime);
	free_node(a);
	free_node(b);
	return 0;
}

int interact(struct Node* a, struct Node* b, Runtime* runtime){
	printf("interact: %s -> %s\n", format_node(a), format_node(b));
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

		case ROOT_TAG:
			return 0;
		case APP_TAG:
			switch (b->tag){
				case NULL_TAG: return erase(b, a, runtime);
				case LAM_TAG: return anihilate(a,b, runtime);
				case SUP_TAG: return commute(a,b, runtime);
			}
		case DUP_TAG:
			switch (b->tag){
				case NULL_TAG: return erase(b, a, runtime);
				case SUP_TAG:
					if (a->label == b->label){
						return anihilate(a,b, runtime);
					}
				case LAM_TAG: return commute(a,b, runtime);
			}
		case NULL_TAG:
		case LAM_TAG:
		case SUP_TAG:
			switch (b->tag){
				case ERA_TAG:case APP_TAG:case DUP_TAG:case ROOT_TAG:
					return interact(b, a, runtime);
			}
	}
	printf("ERROR: Invalid interaction %s -> %s\n", format_node(a), format_node(b));
	return 1;
}

int reduce(Runtime* runtime){

	runtime->stack_top--;
	struct Node* a = (struct Node*)runtime->stack[runtime->stack_top];
	struct Node* b = a->main.location;

	return interact(a, b, runtime);
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


/* BASIC TERMS*/


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

	Port* other_port = get_port(port->location, port->port_number);
	other_port->location = dup;
	other_port->port_number = 0;

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
	Port inner_app = mk_app(f , var(xlam), runtime);
	add_return(xlam, mk_app(f2, inner_app, runtime));
	return flam;
}



/* TESTS */


typedef struct test_error{
	char* error_message;
	struct test_error* next;
} test_error;

struct test_error* test_errors = NULL;


int add_test_error(char* error_message){
	struct test_error* new_error = malloc(sizeof(struct test_error));
	char* error_message_copy = malloc(strlen(error_message) + 1);
	strcpy(error_message_copy, error_message);
	new_error->error_message = error_message_copy;
	new_error->next = test_errors;
	test_errors = new_error;
	return 0;
}



int assert_string_equal(char* a, char* b){
  if (strcmp(a, b) != 0){
		char error_template[] = "Expected %s to be equal to %s";
		char* error_message = malloc(strlen(error_template) + strlen(a) + strlen(b) + 1);
		sprintf(error_message, error_template, a, b);
		add_test_error(error_message);
		return 1;
	}
	return 0;
}

int assert_format(Port port, char* expected){
	char* formatted = format_term(port);
	assert_string_equal(formatted, expected);
	free(formatted);
	return 0;
}


int test_format(Runtime* runtime){
	Port zero = church_zero();
	assert_format(zero, "λ λ x0");
	assert_format(null(), "NULL");
	assert_format(church_true(), "λ λ x1");
	assert_format(church_false(), "λ λ x0");
	assert_format(church_one(runtime), "λ λ (x1 x0)");
	assert_format(church_two(runtime), "λ λ (x1 (x1 x0))");
	return 0;
}



int test_free_term(Runtime* runtime){
	(node_count) = 0;
	Port term = church_two(runtime);
	free_term(term);
	if (node_count != 0){
		add_test_error("Node count is not 0 after free_term");
		return 1;
	}
	return 0;
}


int run_tests(void){
	Runtime runtime = fresh_runtime();
	test_format(&runtime);
	test_free_term(&runtime);
	free_runtime(&runtime);
	if (test_errors != NULL){
		while (test_errors != NULL){
			printf("%s\n", test_errors->error_message);
			test_error* current = test_errors->next;
			free(test_errors->error_message);
			free(test_errors);
			test_errors = current;
		}
		return 1;
	}
	return 0;
}




int main(void) {

	// run_tests();

	Runtime runtime = fresh_runtime();

	Port term = mk_app(mk_id(), mk_id(), &runtime);

	connect_ports(root(), term, &runtime);
	printf("term: %s\n", format_term(term));
	

	int interaction_count = 0;
	execute_stack(&runtime, &interaction_count);

	printf("term: %s\n", format_term(term));
	return 0;


}
