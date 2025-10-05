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




// #define Node_Type_Null 1
// #define Node_Type_Lam 2
// #define Node_Type_App 3
// #define Node_Type_Dup 4
// #define Node_Type_Sup 5
// #define Node_Type_Era 6
// #define Node_Type_Root 7

typedef enum Node_Type{
	Node_Type_Era,
	Node_Type_Null,
	Node_Type_Lam,
	Node_Type_App,
	Node_Type_Dup,
	Node_Type_Sup,
	Node_Type_Root,

	Node_Intermidiate_Var,
}Node_Type;


int node_count = 0;

typedef struct port {
	struct Node* location;
	char port_number;
}Port;

typedef struct Node{
	Node_Type tag;
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





void push_redex(void* value, Runtime* runtime){
	runtime->stack[runtime->stack_top] = value;
	runtime->stack_top++;
}


char get_tag(void* Node){
	return ((struct Node*)Node)->tag;
}

char get_arity(char tag){
	switch (tag){
		case Node_Type_Era:
		case Node_Type_Null:
		case Node_Type_Root:
			return 1;
		default:
			return 3;
		
	}
}

char get_polarity(char tag, char port_number){
	switch (tag){
		case Node_Type_Era:
		case Node_Type_Root:
		return 1;
		case Node_Type_Null:
		case Node_Intermidiate_Var:
			return -1;
		case Node_Type_Lam:
			return port_number == 2 ? 1 : -1;
		case Node_Type_App:
			return port_number == 2 ? -1 : 1;
		case Node_Type_Dup:
			return port_number == 0 ? 1 : -1;
		case Node_Type_Sup:
			return port_number == 0 ? -1 : 1;
	}
	return 0;
}

char* tag_name(Node* node){
	if (node == NULL){
		return "ERR";
	}
	switch (node->tag){
		case Node_Type_Era:
			return "Era";
		case Node_Type_Null:
			return "Nul";
		case Node_Type_Lam:
			return "Lam";
		case Node_Type_App:
			return "App";
		case Node_Type_Dup:
			return "Dup";
		case Node_Type_Sup:
			return "Sup";
		case Node_Type_Root:
			return "Roo";
		case Node_Intermidiate_Var:
			return "Var";
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
		sprintf(buf, "<%s: %s.%d %s.%d %s.%d>",
			tag_name(Node),
			tag_name(Node->main.location), Node->main.port_number,
			tag_name(Node->aux1.location), Node->aux1.port_number,
			tag_name(Node->aux2.location), Node->aux2.port_number);
	}
	return buf;
}

char* format_port(struct port Port){
	char* nodefmt = format_node(Port.location);
	char* buf = malloc(strlen(nodefmt) + 5);
	sprintf(buf, "%s.%d", nodefmt, Port.port_number);
	free(nodefmt);
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
		case Node_Type_Lam:
			if (port.port_number == 1){
				return;
			}
			free_term(node->aux2);
			break;
		case Node_Type_App:
			free_term(node->main);
			free_term(node->aux1);
			break;
		case Node_Type_Dup:
			get_port(node, port.port_number)->location = NULL;
			char other_port_number = 3 - port.port_number;
			if (get_port(node, other_port_number)->location == NULL){
				break;
			}else{
				return;
			}
		default: break;
	}
	free_node(node);
	return;

}

char arity(char tag){
	if (tag == Node_Type_Era || tag == Node_Type_Null){
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
	if (an == 0 && bn == 0 && a->tag != Node_Type_Root && b->tag != Node_Type_Root){
		push_redex(a, runtime);
	}
	return 0;
}

int connect_ports(struct port a, struct port b, Runtime* runtime){
	return connect_nodes(a.location, b.location, a.port_number, b.port_number, runtime);
}


Port other_port(Port port){
	Port* myport = get_port(port.location, port.port_number);
	return (Port){.location = myport->location, .port_number = myport->port_number};
}


int replace_port(struct port previous, struct port new, Runtime* runtime){
	Port target = other_port(previous);
	return connect_ports(target, new, runtime);
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



int port_equals(Port a, Port b){
	return a.location == b.location && a.port_number == b.port_number;
}

int check_port(Port port){
	Port otherother = other_port(other_port(port));
	return 1-port_equals(otherother, port);
}


// int check_port(Node* location, char port_number){

// 	Node* node_a = location;
// 	Port* port_a = get_port(node_a, port_number);

// 	Node* node_b = port_a->location;
// 	Port* port_b = get_port(node_b, port_a->port_number);
	
// 	if (port_b->location != location){
// 		return 1;
// 	}

// 	if (port_b->port_number != port_number){
// 		return 1;
// 	}

// 	return 0;
// }


int check_node(struct Node* Node){

	if (Node->tag == 0){
		printf("Check Node Error: Invalid tag");
		return 1;
	}
	if (Node->main.location == NULL){
		printf("Error: Main port is NULL for Node %s\n", format_node(Node));
		return 1;
	}

	if (check_port(Node->main)){
		printf("Error: Invalid main port for Node %s\n", format_node(Node));
		return 1;
	}

	if (get_arity(Node->tag) == 1){
		return 0;
	}

	if (check_port(Node->aux1)){
		printf("Error: Invalid aux1 port for Node %s\n", format_node(Node));
		return 1;
	}

	if (check_port(Node->aux2)){
		printf("Error: Invalid aux2 port for Node %s\n", format_node(Node));
		return 1;
	}
	return 0;

}

struct port era(void){
	struct Node* Node = new_node(Node_Type_Era);
	return (struct port){
		.location = Node,
		.port_number = 0
	};
}

struct port null(void){
	struct Node* Node = new_node(Node_Type_Null);
	return (struct port){
		.location = Node,
		.port_number = 0
	};
}

struct port root(void){
	struct Node* Node = new_node(Node_Type_Root);
	return (struct port){
		.location = Node,
		.port_number = 0
	};
}

Port mk_root(Port port, Runtime* runtime){
	Port rt = root();
	connect_ports(port, rt, runtime);
	return rt;
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
	struct port r = bin(a, b, Node_Type_Dup);
	r.location->label = label;
	return r;
}

int idup_ctr = (1<<8)-1;

struct port ndup(Port a, Port b){
	return dup(a, b, idup_ctr--);
}

struct port sup(struct port a, struct port b, char label){
	struct port r = bin(a, b, Node_Type_Sup);
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
	if (lam.location->tag != Node_Type_Lam){
		printf("Error: Node is not a lambda\n");
	}
	return (Port){.location = lam.location, .port_number = 1};
}

Port mk_lam(void){
	return mk_main(new_node(Node_Type_Lam));
}

int add_return(struct port a, struct port b){
	return add_aux(a.location, 2, b);
}

Port mk_app(Port fn, Port arg, Runtime* runtime){
	Node * app = new_node(Node_Type_App);
	add_aux(app, 1, arg);
	connect_ports(fn, mk_main(app), runtime);
	return mk_aux_2(app);
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




typedef struct name_ctx{

	Node* node;
	struct name_ctx* prev;
}name_ctx;

char* do_format_term(Port port, name_ctx* ctx){
	// printf("do_format_term: %s from port %d\n", format_node(port.location), port.port_number);
	// check_node(port.location);

	// 
	switch (port.location->tag){
		case Node_Type_Lam:
			if (port.port_number == 1){

				char myd = 0;
				name_ctx* current = ctx;
				while (current != NULL && current->node != port.location){
					current = current->prev;
					myd++;
				}
				if (current == NULL){
					// printf("Error: Node %s not found in ctx\n", format_node(port.location));
					return string_malloc("x?");
				}
				char buf[32];
				snprintf(buf, sizeof(buf), "x%d", myd);
				return string_malloc(buf);

			}
			name_ctx* new_ctx = malloc(sizeof(name_ctx));
			new_ctx->node = port.location;
			new_ctx->prev = ctx;
			char* aux2 = do_format_term(port.location->aux2, new_ctx);
			free(new_ctx);
			return string_concat(string_malloc("λ."),  aux2);
		case Node_Type_App:{
				char* sa = do_format_term(port.location->main, ctx);
				char* sb = do_format_term(port.location->aux1, ctx);
				char* buf = malloc(strlen(sa) + strlen(sb) + 3);
				sprintf(buf, "(%s %s)", sa, sb);
				free(sa);
				free(sb);
				return buf;
			}
		case Node_Type_Dup:{
			char* f0 = do_format_term(port.location->main, ctx);
			// char* buf = malloc(strlen(f0) + 3);
			// sprintf(buf, "&[%s]", f0);
			// free(f0);
			// return buf;

			return f0;
		}
		case Node_Type_Sup:
			return string_malloc("&{}");
		case Node_Type_Null:
			return string_malloc("NULL");
		
		// default: return string_malloc("UNK");

	}
	// return string_malloc("UNKOWN TERM");
	// return 
	char*errmsg = "UNKOWN TERM %s";
	char* buf = malloc(strlen(errmsg) + 40);
	sprintf(buf, "UNKOWN TERM %s", tag_name(port.location));
	return buf;
}


char* format_term(Port port){
	name_ctx* ctx = NULL;
	return do_format_term(port, ctx);
}



/* INTERACTIONS */


int interaction_void(struct Node* a, struct Node* b){
	free_node(a);
	free_node(b);
	return 0;
}

int erase(struct Node* e, struct Node* c, Runtime* runtime){
	struct Node* e2 = new_node(e->tag);
	if (connect_nodes(e, c->aux1.location, 0, c->aux1.port_number, runtime) ||
		connect_nodes(e2, c->aux2.location, 0, c->aux2.port_number, runtime)
	) {
		return 1;
	}
	return 0;
}

int commute(struct Node* a, struct Node* b, Runtime* runtime){
	struct Node* a2 = new_node(a->tag);
	a2->label = a->label;
	struct Node* b2 = new_node(b->tag);
	b2->label = b->label;
	if (
		connect_nodes(b->aux1.location, a, b->aux1.port_number, 0, runtime) || 
		connect_nodes(b->aux2.location, a2, b->aux2.port_number, 0, runtime) ||
		connect_nodes(a->aux1.location, b2, a->aux1.port_number, 0, runtime) ||
		connect_nodes(a->aux2.location, b, a->aux2.port_number, 0, runtime) ||
		connect_nodes(a, b, 2, 1, runtime) ||
		connect_nodes(a, b2, 1, 1, runtime) ||
		connect_nodes(a2, b, 2, 2, runtime) ||
		connect_nodes(a2, b2, 1, 2, runtime)
	) {
		return 1;
	}
	return 0;
}

int dup_lam(struct Node* dup, struct Node* lam, Runtime* runtime){
	struct Node* sup = new_node(Node_Type_Sup);
	sup->label = dup->label;
	struct Node* lam2 = new_node(Node_Type_Lam);

	if (
		replace_port(mk_aux_1(lam), mk_main(sup), runtime)||
		replace_port(mk_aux_2(lam), mk_main(dup), runtime)||
		replace_port(mk_aux_1(dup), mk_main(lam), runtime)||
		replace_port(mk_aux_2(dup), mk_main(lam2), runtime)||
		
		connect_nodes(lam, sup, 1, 1, runtime)||
		connect_nodes(lam2, sup, 1, 2, runtime)||
		connect_nodes(lam, dup, 2, 1, runtime)||
		connect_nodes(lam2, dup, 2, 2, runtime)
	){
		return 1;

	}
	if (

		check_node(dup)||
		check_node(sup)||
		check_node(lam)||
		check_node(lam2)
	) {
		return 1;
	}
	return 0;
}



int anihilate(struct Node* a, struct Node* b, Runtime* runtime){

	// printf("anihilate: %s -> %s\n", format_node(a), format_node(b));
	connect_nodes(a->aux1.location, b->aux1.location, a->aux1.port_number, b->aux1.port_number, runtime);
	connect_nodes(a->aux2.location, b->aux2.location, a->aux2.port_number, b->aux2.port_number, runtime);
	free_node(a);
	free_node(b);

	return 0;
}

int interact(struct Node* a, struct Node* b, Runtime* runtime){
	if (check_node(a)){
		printf("Error: Invalid node %s\n", format_node(a));
		return 1;
	}
	if (check_node(b)){
		printf("Error: Invalid node %s\n", format_node(b));
		return 1;
	}

	switch (a->tag){

		case Node_Type_Null:
		case Node_Type_Lam:
		case Node_Type_Sup:{

			switch (b->tag){
				case Node_Type_Era:case Node_Type_App:case Node_Type_Dup:case Node_Type_Root:
				  return interact(b, a, runtime);	
				default: break;
			}
		}
		default:break;
	}
	
	// printf("interact: %s -> %s\n", format_node(a), format_node(b));

	switch (a->tag){
		case Node_Type_Era:
			switch (b->tag){
				case Node_Type_Null:return interaction_void(a, b);
				case Node_Type_Lam:
					if (b->aux1.location == b->aux2.location){
						return interaction_void(a,b);
					}
				case Node_Type_Sup: return erase(a, b, runtime);
				default: break;
			}

		case Node_Type_Root:
			printf("ROOT INTERACTION?\n");
			return 0;
		case Node_Type_App:
			switch (b->tag){
				case Node_Type_Null: return erase(b, a, runtime);
				case Node_Type_Lam:
					return anihilate(a,b, runtime);
				case Node_Type_Sup: return commute(a,b, runtime);
				default: break;
			}
		case Node_Type_Dup:
			switch (b->tag){
				case Node_Type_Null: return erase(b, a, runtime);
				case Node_Type_Sup:
					if (a->label == b->label){
						return anihilate(a,b, runtime);
					}
				case Node_Type_Lam: return dup_lam(a,b, runtime);
				default: break;
			}
		default: break;
	}
	printf("ERROR: Invalid interaction %s -> %s\n", format_node(a), format_node(b));
	return 1;
}

int reduce(Runtime* runtime){

	runtime->stack_top--;
	struct Node* a = (struct Node*)runtime->stack[runtime->stack_top];
	struct Node* b = a->main.location;

	int res = interact(a, b, runtime);
	if (res){
		printf("Reduce Error\n");
	}

	return res;
}



int execute_stack(Runtime* runtime, int* interaction_count){
	while (runtime->stack_top > 0){
		if (reduce(runtime)){
			printf("Reduce Error\n");
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
	Node* dup = new_node(Node_Type_Dup);
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
	assert_format(zero, "λ.λ.x0");
	assert_format(null(), "NULL");
	assert_format(church_true(), "λ.λ.x1");
	assert_format(church_false(), "λ.λ.x0");
	assert_format(church_one(runtime), "λ.λ.(x1 x0)");
	assert_format(church_two(runtime), "λ.λ.(x1 (x1 x0))");
	return 0;
}



int assert_reduction(Port term, char* expected, Runtime* runtime){
	Port rt = mk_root(term, runtime);
	int interaction_count = 0;
	// printf("assert_reduction: %s -> %s\n", format_term(term), expected);
	execute_stack(runtime, &interaction_count);

	char * res = format_term(other_port(rt));
	// printf("assert_reduction: %s\n", res);
	free(res);
	assert_format(other_port(rt), expected);
	return 0;
}

int test_reduction(Runtime* runtime){
	assert_reduction(church_zero(), "λ.λ.x0", runtime);
	assert_reduction(church_one(runtime), "λ.λ.(x1 x0)", runtime);
	assert_reduction(mk_app(mk_id(), mk_id(), runtime), "λ.x0", runtime);
	// assert_reduction(mk_app(church_two(runtime), mk_id(), runtime), "λ.x0", runtime);
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
	test_reduction(&runtime);
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

	run_tests();
	return 0;


}
