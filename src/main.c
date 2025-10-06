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

Node* stack[1000];
Runtime runtime = (Runtime){.stack = stack, .stack_top = 0};

void fresh_runtime(void){
	runtime.stack = stack;
	runtime.stack_top = 0;
}

char get_tag(void* Node){
	return ((struct Node*)Node)->tag;
}

char get_arity(char tag){
	switch (tag){
		case Node_Type_Era:
		case Node_Type_Null:
		case Node_Type_Root:
		case Node_Intermidiate_Var:
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
	return "UNK tag";
}

char* format_node(struct Node* Node){

	// printf("format_node: %s\n", tag_name(Node));
	if (Node == NULL){
		return "NULL";
	}
	char* buf = malloc(100);
	if (get_arity(Node->tag) == 1){
		sprintf(buf, "<%s: %s>", tag_name(Node), tag_name(Node->main.location));
	}else{

		char * label;

		if (Node->tag == Node_Type_Dup || Node->tag == Node_Type_Sup){
			label = malloc(10);
			sprintf(label, " %d", Node->label);
		}else{
			label = "";
		}

		sprintf(buf, "<%s%s: %s.%d %s.%d %s.%d>",
			tag_name(Node),
			label,
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


void push_redex(Node* value){
	// printf("push_redex: %s\n", format_node(value));
	runtime.stack[runtime.stack_top] = value;
	runtime.stack_top++;
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

int connect_nodes(struct Node* a, struct Node* b, char an, char bn){

	// printf("connect_nodes\n");
	if (get_polarity(a->tag, an) == get_polarity(b->tag, bn)){
		printf("Error: Polarities do not match for ports %s.%d and %s.%d\n", format_node(a), an, format_node(b), bn);
		exit(1);
	}
	if (a == b && (an == 0 || bn == 0)){
		printf("Error: Cannot loop on main port: %s\n", format_node(a));
		exit(1);
	}
	if (set_port(a, an, b, bn)){
		return 1;
	}
	if (set_port(b, bn, a, an)){
		return 1;
	}
	if (an == 0 && bn == 0 && a->tag != Node_Type_Root && b->tag != Node_Type_Root && a->tag != Node_Intermidiate_Var && b->tag != Node_Intermidiate_Var){
		push_redex(a);
	}
	return 0;
}

int connect_ports(struct port a, struct port b){
    return connect_nodes(a.location, b.location, a.port_number, b.port_number);
}


Port other_port(Port port){
	Port* myport = get_port(port.location, port.port_number);
	return (Port){.location = myport->location, .port_number = myport->port_number};
}


int replace_port(struct port previous, struct port new){
	Port target = other_port(previous);
    return connect_ports(target, new);
}


int add_aux(struct Node* Node, char port_number, struct port a){
	if (port_number==0){
		printf("Not an aux port\n");
		return 1;
	}
	return connect_nodes(Node, a.location, port_number, a.port_number);
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


int check_node(struct Node* Node){

	if (Node->tag == 0){
		printf("Check Node Error: Invalid tag\n");
		exit(1);
	}
	if (Node->main.location == NULL){
		printf("Error: Main port is NULL for Node %s\n", format_node(Node));
		exit(1);
	}

	if (Node->main.location == Node){
		printf("Error: Main port is self for Node %s\n", format_node(Node));
		exit(1);
	}

	if (check_port(Node->main)){
		printf("Error: Invalid main port for Node %s\n", format_node(Node));
		exit(1);
	}

	if (get_arity(Node->tag) == 1){
		return 0;
	}

	if (check_port(Node->aux1)){
		printf("Error: Invalid aux1 port for Node %s\n", format_node(Node));
		exit(1);
	}

	if (check_port(Node->aux2)){
		printf("Error: Invalid aux2 port for Node %s\n", format_node(Node));
		exit(1);
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

Port mk_root(Port port){
	Port rt = root();
    connect_ports(port, rt);
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

int idup_ctr = 0;

struct port ndup(Port a, Port b){
	return dup(a, b, idup_ctr++);
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
		exit(1);
	}
	return (Port){.location = lam.location, .port_number = 1};
}

Port mk_lam(void){
	return mk_main(new_node(Node_Type_Lam));
}

int add_return(struct port a, struct port b){
	return add_aux(a.location, 2, b);
}

Port app(Port f, Port x){
	Node * app = new_node(Node_Type_App);
	add_aux(app, 1, x);
    connect_ports(f, mk_main(app));
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
	switch (port.location->tag){
		case Node_Type_Lam:{

			if (port.port_number == 1){
				
				char myd = 0;
				name_ctx* current = ctx;
				while (current != NULL && current->node != port.location){
					current = current->prev;
					myd++;
				}
				if (current == NULL){
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
		}
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
			char* buf = malloc(strlen(f0) + 100);

			printf("label: %d\n", port.location->label);

			sprintf(buf, "&%d(%s)", port.location->label, f0);
			// sprintf(buf, "&%d(%s)", 1234, f0);
			// printf("buf: %s\n", buf);
			free(f0);
			return buf;
			// return f0;
		}
		case Node_Type_Sup:{
			// char* buf = malloc(10);
			// sprintf(buf, "&%d{}", port.location->label);
			// return buf;

			return string_malloc("&{}");
		}
		case Node_Type_Null:
			return string_malloc("NULL");
		case Node_Intermidiate_Var:
		case Node_Type_Root: return string_malloc("ROOT TERM?");
		case Node_Type_Era: return string_malloc("ERA TERM?");

		default:
			printf("Error: Invalid tag %d\n", port.location->tag);
			exit(1);

	}

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

int erase(struct Node* e, struct Node* c){
	struct Node* e2 = new_node(e->tag);
    if (connect_nodes(e, c->aux1.location, 0, c->aux1.port_number) ||
        connect_nodes(e2, c->aux2.location, 0, c->aux2.port_number)
	) {
		exit(1);
	}
	return 0;
}

int commute(struct Node* a, struct Node* b){
	struct Node* a2 = new_node(a->tag);
	a2->label = a->label;
	struct Node* b2 = new_node(b->tag);
	b2->label = b->label;
	connect_nodes(b->aux1.location, a, b->aux1.port_number, 0) ;
	connect_nodes(b->aux2.location, a2, b->aux2.port_number, 0);
	connect_nodes(a->aux1.location, b2, a->aux1.port_number, 0);
	connect_nodes(a->aux2.location, b, a->aux2.port_number, 0);
	connect_nodes(a, b, 2, 1);
	connect_nodes(a, b2, 1, 1);
	connect_nodes(a2, b, 2, 2);
	connect_nodes(a2, b2, 1, 2);
	return 0;
}

int dup_lam(struct Node* a, struct Node* b){
	struct Node* a2 = new_node(Node_Type_Sup);
	struct Node* b2 = new_node(Node_Type_Lam);
	a2->label = a->label;

	replace_port(mk_aux_1(b), mk_main(a2));
	replace_port(mk_aux_2(b), mk_main(a));
	replace_port(mk_aux_1(a), mk_main(b));
	replace_port(mk_aux_2(a), mk_main(b2));

	connect_nodes(b, a2, 1, 1);
	connect_nodes(b2, a2, 1, 2);
	connect_nodes(b, a, 2, 1);
	connect_nodes(b2, a, 2, 2);

	check_node(a);
	check_node(a2);
	check_node(b);
	check_node(b2);

	// printf("dup_lam: checked.\n");s

	return 0;
}

int app_sup(struct Node* a, struct Node* b){
	struct Node* a2 = new_node(Node_Type_App);
	struct Node* b2 = new_node(Node_Type_Dup);
	b2->label = b->label;

    replace_port(mk_aux_1(b), mk_main(a));
    replace_port(mk_aux_2(b), mk_main(a2));
    replace_port(mk_aux_1(a), mk_main(b2));
    replace_port(mk_aux_2(a), mk_main(b));
	
    connect_nodes(a, b2, 1, 1);
    connect_nodes(a2, b2, 1, 2);
    connect_nodes(a, b, 2, 1);
    connect_nodes(a2, b, 2, 2);

	check_node(a);
	check_node(b2);
	check_node(b);
	check_node(a2);

	return 0;
}



int anihilate(struct Node* a, struct Node* b){

	// printf("a aux2: %s\n", format_node(a->aux2.location));
	// printf("b aux2: %s\n", format_node(b->aux2.location));
    connect_nodes(a->aux1.location, b->aux1.location, a->aux1.port_number, b->aux1.port_number);
    connect_nodes(a->aux2.location, b->aux2.location, a->aux2.port_number, b->aux2.port_number);
	free_node(a);
	free_node(b);

	return 0;
}

int interact(struct Node* a, struct Node* b){



	switch (a->tag){

		case Node_Type_Null:
		case Node_Type_Lam:
		case Node_Type_Sup:{

			switch (b->tag){
				case Node_Type_Era:case Node_Type_App:case Node_Type_Dup:case Node_Type_Root:
					return interact(b, a); 
				default: break;
			}
		}
		default:break;
	}

	check_node(a);
	check_node(b);

	// printf("interact: %s <> %s\n", format_node(a), format_node(b));
	switch (a->tag){
		case Node_Type_Era:
			switch (b->tag){
                case Node_Type_Null:return interaction_void(a, b);
				case Node_Type_Lam:
					if (b->aux1.location == b->aux2.location){
						return interaction_void(a,b);
					}
                case Node_Type_Sup: return erase(a, b);
				default: break;
			}

		case Node_Type_Root:
			printf("ROOT INTERACTION?\n");
			return 0;
		case Node_Type_App:
			switch (b->tag){
				case Node_Type_Null: return erase(b, a);
				case Node_Type_Lam:
						return anihilate(a,b);
				case Node_Type_Sup: return app_sup(a,b);
				default: break;
			}
		case Node_Type_Dup:
			switch (b->tag){
				case Node_Type_Null: return erase(b, a);
				case Node_Type_Sup:
					if (a->label == b->label){
						return anihilate(a,b);
					}else{
						return commute(a,b);
					}
				case Node_Type_Lam: return dup_lam(a,b);
				default: break;
			}
		default: break;
	}
	printf("ERROR: Invalid interaction %s -> %s\n", format_node(a), format_node(b));
	exit(1);
}




void add_use(Node* lam, Port x){

	/*
		add a use location for the lambda.
	  lam: lambda that provides the binding for x
		x: use of the binding. not the var but the use of the var.
	*/
	Port* bindr = get_port(lam, 1);
	
	Node* prev_user = bindr->location;
	if (get_tag(prev_user) == Node_Type_Era){
		printf("add_use: prev_user is Era\n");

		connect_ports((Port){.location = lam, .port_number = 1}, x);
		free_node(prev_user);
		return;
	}
	
	printf("add_use: %s %s \n", format_node(lam), format_port(x));

	Port dup = mk_main(new_node(Node_Type_Dup));
	dup.location->label = idup_ctr ++ ;
	add_auxs(dup.location, (Port){.location = prev_user, .port_number = bindr->port_number}, x);
	connect_ports(dup, (Port){.location = lam, .port_number = 1});
}

void connect_intermidiate_vars(Port port, name_ctx* ctx){

	// printf("port.port_number: %d\n", port.port_number);
	// printf("ctx NULL: %d\n", ctx == NULL);

	Node* node = port.location;

	switch (node->tag){
		case Node_Type_Lam:{
			if (port.port_number != 0){
				break;
			}
			name_ctx* new_ctx = malloc(sizeof(name_ctx));
			new_ctx->node = node;
			new_ctx->prev = ctx;
			connect_intermidiate_vars(node->aux2, new_ctx);
			free(new_ctx);
			break;
		}
		case Node_Type_App:{
			if (port.port_number != 2){
				break;
			}
			connect_intermidiate_vars(node->main, ctx);
			connect_intermidiate_vars(node->aux1, ctx);
			break;
		}
		case Node_Type_Dup:{
			connect_intermidiate_vars(node->main, ctx);
		}

		case Node_Intermidiate_Var:{

			name_ctx* current = ctx;
			char ctr = node->label;


			while (current != NULL && ctr > 0){
				current = current->prev;
				ctr--;
			}


			if (current != NULL){
				add_use(current->node, node->main);
				free_node(node);
			}
		}
		default: break;
	}

}


Port lam(Port b){
	Node* node = new_node(Node_Type_Lam);
	add_auxs(node, era(), b);
	Port res = mk_main(node);
	connect_intermidiate_vars(res, NULL);
	printf("lam: %s\n", format_node(node));
	return res;
}



Port x(char n){
	Node* node = new_node(Node_Intermidiate_Var);
	node->label = n;
	return mk_main(node);
}




Port normalize(Port port){
	Port rt = mk_root(port);
	int interaction_count = 0;
	printf("normalize: %s\n", format_term(port));

	while (runtime.stack_top > 0){
		runtime.stack_top--;
		struct Node* a = (struct Node*)runtime.stack[runtime.stack_top];
		struct Node* b = a->main.location;
		interact(a, b);	
		interaction_count++;
		printf("term: %s\n", format_term(other_port(rt)));
	}
	printf("normalize: done\n");
	return other_port(rt);
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
	connect_ports(var(bod), era());
	return lam;
}

Port church_false(void){
	return lam(lam(x(0)));
}

Port church_zero(void){
	return church_false();
}


Port church_one(void){
	return lam(lam(app(x(1), x(0))));
}

Port church_two(void){
	return lam(lam(app(x(1), app(x(1), x(0)))));
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


int test_format(void){
	Port zero = church_zero();
	assert_format(zero, "λ.λ.x0");
	assert_format(null(), "NULL");
	assert_format(church_true(), "λ.λ.x1");
	assert_format(church_false(), "λ.λ.x0");
	assert_format(church_one(), "λ.λ.(x1 x0)");
	assert_format(church_two(), "λ.λ.(x1 (x1 x0))");
	return 0;
}

int assert_reduction(Port term, char* expected){


	Port ret = normalize(term);

	char * res = format_term(ret);
	printf("assert_reduction: %s\n", res);
	free(res);
	assert_format(ret, expected);
	return 0;
}

int test_reduction(void){
	assert_reduction(church_zero(), "λ.λ.x0");
	// assert_reduction(church_one(), "λ.λ.(x1 x0)");
	// assert_reduction(app(mk_id(), mk_id()), "λ.x0");
	// assert_reduction(app(church_two(), mk_id()), "λ.x0");
	return 0;
}


int test_free_term(void){
	fresh_runtime();
	free_term(church_two());
	if (runtime.stack_top != 0){
		add_test_error("Stack is not empty after free_term");
		return 1;
	}
	return 0;
}


int run_tests(void){
    fresh_runtime();
    test_format();
    test_free_term();
    test_reduction();
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


char* format_root(Port root){
	return format_term(other_port(root));
}  



int main(void) {

	// run_tests();

	// normalize(church_two());


	// normalize(app(church_two(), mk_id()));

	// normalize(app(church_two(), mk_id()));
	normalize(app(church_two(), church_two()));



	return 0;

}
