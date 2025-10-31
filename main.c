
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>



static jmp_buf segfault_jmp;
static volatile sig_atomic_t segfault_occurred = 0;

void segfault_handler(int sig) {
  segfault_occurred = 1;
  longjmp(segfault_jmp, 1);
}

int DEBUG = 1;

void set_debug(int debug){
  DEBUG = debug;
}

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"

void debug(char* content){
  if (DEBUG){
    printf(YELLOW "DEBUG: %s" RESET, content);
  }
}

void error(char* content){
  fprintf(stderr, RED "Error: %s\n" RESET, content);
  exit(1);
}

typedef enum Tag{
  Tag_App,
  Tag_Lam,
  Tag_Sup,
  Tag_Dup,
  Tag_Dup2,
  Tag_Null,
  Tag_Var,
  Tag_Freed,
}Tag;

typedef struct Node{
  Tag tag;
  int label;
  struct Node* s0;
  struct Node* s1;
}Node;

#define MAX_NODES 1<<20

typedef struct Runtime{
  Node nodes[MAX_NODES];
  int empty_index;
  int node_ctr;
  Node* free_list;
  int steps;
} Runtime;

Runtime* runtime;

char* tag_names[8] = { "App", "Lam", "Sup", "Dup", "Dup2", "Null", "Var", "Freed" };

char * tag_name(int tag){
  return tag_names[tag];
}


void check_node(Node* node);

Node* new_node(Tag tag, int label, Node* s0, Node* s1){
  Node* node = NULL;
  if (runtime->free_list != NULL){
    node = runtime->free_list;
    runtime->free_list = node->s0;
  }else{
    node = &runtime->nodes[runtime-> empty_index ++ ];
    if (runtime->empty_index >= MAX_NODES){
      error("Error: MAX_NODES reached\n");
    }
    runtime->node_ctr ++;
  }
  runtime->node_ctr ++;
  node->tag = tag;
  node->label = label;
  node->s0 = s0;
  node->s1 = s1;
  if (DEBUG >= 2) printf("new node: %s %p\n", tag_name(tag), node);
  return node;
}



void free_node(Node* node){
  if (DEBUG && node->tag == Tag_Freed){
    printf("Error: Node %p is already freed\n", node);
    exit(1);
  }
  if (DEBUG >= 2){
    printf("free node %p ", node);
    printf("tag: %s ", tag_name(node->tag));
  }
  node->tag = Tag_Freed;
  runtime->node_ctr --;
  node->s0 = runtime->free_list;
  runtime->free_list = node;
}

typedef struct SQueue{
  Node* node;
  int s0;
  int s1;
  struct SQueue* next;
} S_Queue;

typedef struct SearchStack{
  Node* node;
  struct SearchStack* next;
} SearchStack;

/* SERIALIZATION */

int _enqueue(S_Queue* queue, Node* node, int * ctr){
  if (node == NULL) return 0;

  int n = 0;
  while (1){
    if (queue->node == node) return n + 1;
    n ++ ;
    if (queue->next == NULL) break;
    queue = queue->next;
  }
  
  S_Queue* new_node = malloc(sizeof(S_Queue));
  new_node->node = node;
  new_node->next = NULL;
  queue->next = new_node;
  (*ctr)++;

  return n + 1;
}


int* serialize(Node* node){

  S_Queue* queue = malloc(sizeof(S_Queue));
  queue->node = node;
  queue->next = NULL;
  S_Queue* current = queue;
  int ctr = 1;

  while (current != NULL){
    Node* node = current->node;
    current -> s0 = _enqueue(queue, node->s0, &ctr);
    current -> s1 = _enqueue(queue, node->s1, &ctr);
    current = current->next;
  }

  int* result = malloc(sizeof(int) * (ctr + 1) * 4);
  current = queue;

  result[0] = ctr;
  ctr = 1;

  if (DEBUG >= 2) printf("SERIALIZE: %d nodes\n", result[0]);

  while (1){
    result[ctr] = current->node->tag;
    result[ctr + 1] = current->node->label;
    result[ctr + 2] = current->s0;
    result[ctr + 3] = current->s1;
    
    if (DEBUG >= 2) printf("  [%d] tag=%s label=%d s0=%d s1=%d\n", (ctr-1)/4 + 1, tag_name(current->node->tag), current->node->label, current->s0, current->s1);
    ctr += 4;
    S_Queue* prev = current;
    current = current->next;
    if (current == NULL) break;
    free(prev);
  }
  return result;
}

Node** mk_dup(Node* target, int label){
  Node* dup1 = new_node(Tag_Dup, label, target, NULL);
  Node* dup2 = new_node(Tag_Dup2, label, target, dup1);
  dup1->s1 = dup2;

  Node**res = malloc(sizeof(Node*) * 2);
  res[0] = dup1;
  res[1] = dup2;
  return res;
}

Node* sup(Node* a, Node* b, int label){
  if (a == NULL) a = new_node(Tag_Null, 0, NULL, NULL);
  if (b == NULL) b = new_node(Tag_Null, 0, NULL, NULL);
  return new_node(Tag_Sup, label, a, b);
}


void check_null(void* ptr, char* tag_name){
  if (ptr == NULL){
    printf("Error: %s is NULL\n", tag_name);
    exit(1);
  }
}


void check_node(Node* node){
  if (node == NULL){
    printf("Error: node is NULL\n");
    exit(1);
  }
  switch (node->tag){
    case Tag_Dup:
      if (node->s1 != NULL && node->s1->tag != Tag_Dup2){
        printf("Error: Dup->s0->tag == %s instead of Dup\n", tag_name(node->s0->tag));
        exit(1);
      }
      check_null(node->s0, "Dup->s0");
      break;
    case Tag_Dup2:
      if (node->s1 != NULL && node->s1->tag != Tag_Dup){
        printf("Error: Dup2->s0->tag == %s instead of Dup\n", tag_name(node->s0->tag));
        exit(1);
      }
      check_null(node->s0, "Dup->s0");
      break;
    case Tag_Var:
      if (node->s0->tag != Tag_Lam){
        printf("Error: Var->s0->tag %p == %s %p instead of Lam\n",node, tag_name(node->s0->tag), node->s0);
        exit(1);
      }
      check_null(node->s0, "Var->s0");
      break;
    case Tag_Lam:
      if (node->s1 != NULL && node->s1->tag != Tag_Var){
        printf(RED "Error : Lam %p ->s1->tag == %s %p instead of Var\n" RESET, node, tag_name(node->s1->tag), node->s1);
        exit(1);
      }
      check_null(node->s0, "Lam->s0");
      break;
    case Tag_Sup:
    case Tag_App:
      check_null(node->s0, "App->s0");
      check_null(node->s1, "App->s1");
      break;
    case Tag_Null:break;
    case Tag_Freed: printf(RED "Node %p is freed\n" RESET, node); exit(1);
  }
}


void erase(Node* node);



void move(Node* src, Node* dst){
  if (dst == NULL){
    erase(src);
    return;
  }
  if (DEBUG) printf("move %s %p -> %p\n", tag_name(src->tag), src, dst);

  dst->tag = src->tag;
  dst->s0 = src->s0;
  dst->s1 = src->s1;

  dst->label = src->label;
  if (src->tag == Tag_Var){
    if (src->s0->tag != Tag_Lam) error("Error: Invalid tag for lam in move\n");
    src->s0->s1 = dst;
  }
  if (src->tag == Tag_Lam && src->s1 != NULL){
    if (src->s1->tag != Tag_Var) error("Error: Invalid tag for var in move\n");
    src->s1->s0 = dst;
  }
  if (src->tag == Tag_Dup || src->tag == Tag_Dup2){
    if (dst->s1 != NULL)dst->s1->s1 = dst;
  }
  free_node(src);
}

void erase(Node* node){
  switch (node->tag){
    case Tag_App:
    case Tag_Sup: erase(node->s1);
    case Tag_Lam: erase(node->s0); break;
    case Tag_Var: break;
    case Tag_Dup:
    case Tag_Dup2:{
      if (node->s1 == NULL) erase(node->s0);
      else node->s1->s1 = NULL;
      break;
    }
    case Tag_Null: break;
    case Tag_Freed: error("Node is freed");
  };
  free_node(node);
}



int APP_LAM(Node* App, Node* Lam){

  Node* arg = App->s1;
  Node* var = Lam->s1;
  Node* body = Lam->s0;

  if (var != NULL){
    move(arg, var);
    move(body, App);
    if (Lam->s1 != NULL && Lam->s1->s0 == Lam) {printf("cannot free lam %p it has a var\n", Lam); exit(1);}
  }else{
    move(body, App);
    erase(arg);
  }

  free_node(Lam);
  return 1;
}

int APP_SUP(Node* App, Node* Sup){

  Node** dups = malloc(sizeof(Node*) * 2);

  dups[0] = new_node(Tag_Dup, Sup->label, App->s1, NULL);
  dups[1] = new_node(Tag_Dup2, Sup->label, App->s1, dups[0]);

  dups[0]->s1 = dups[1];
  
  move(sup(new_node(Tag_App, 0, Sup->s0, dups[0]), new_node(Tag_App, 0, Sup->s1, dups[1]), Sup->label), App);

  free(dups);

  return 1;
}


int APP_NULL(Node* App, Node* Null){
  erase(App->s1);
  move(Null, App);
  return 1;
}



int DUP_LAM(Node* dup, Node* Lam){
  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;

  if (DEBUG) printf("da:%p db:%p lam:%p var:%p\n",  da, db, Lam, Lam->s1);

  int label = da == NULL ? db->label : da->label;
  Node** dbody = mk_dup(Lam->s0, label);

  Node* vara = NULL;
  Node* varb = NULL;
  Node* funa = NULL;
  Node* funb = NULL;

  if (da != NULL){
    funa = new_node(Tag_Lam,0, NULL, NULL);
    vara = new_node(Tag_Var, 0, NULL, NULL);
    funa->s0 = dbody[0];
    vara->s0 = funa;
    funa->s1 = vara;
    move(funa, da);
  }

  if (db != NULL){
    funb = new_node(Tag_Lam,0, NULL, NULL);
    varb = new_node(Tag_Var, 0, NULL, NULL);
    funb->s0 = dbody[1];
    varb->s0 = funb;
    funb->s1 = varb;
    move(funb, db);
  }

  if (Lam->s1 != NULL){
    move(sup(vara, varb, label), Lam->s1);
  }
  return 1;
}

int DUP_SUP(Node* dup, Node* Sup){

  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;
  int label = da == NULL ? db->label : da->label;
  if (Sup->label == label){
    if (Sup->s0 == db || Sup->s1 == db){
      move(Sup->s1, db);
      move(Sup->s0, da);
    }else{
      move(Sup->s0, da);
      move(Sup->s1, db);
    }
  } else {
    Node** dup1 = mk_dup(Sup->s0, label);
    Node** dup2 = mk_dup(Sup->s1, label);
    move(sup(dup1[0], dup2[0], Sup->label), da);
    move(sup(dup1[1], dup2[1], Sup->label), db);
    free(dup1);
    free(dup2);
  }

  free_node(Sup);
  return 1;
}


int DUP_NULL(Node* dup, Node* Null){
  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;
  da->tag = Tag_Null;
  move(Null, db);
  return 1;
}

int fuel = 0;

int handle_redex(Node* term, Node* other){

  if (fuel <= runtime->steps) return 0;
  int (*handler)(Node*, Node*) = NULL;
  if (term->tag == Tag_App)
    handler = other->tag == Tag_Lam ? APP_LAM : other->tag == Tag_Sup ? APP_SUP : other->tag == Tag_Null ? APP_NULL : NULL;
  else if (term->tag == Tag_Dup || term->tag == Tag_Dup2){
    handler = other->tag == Tag_Lam ? DUP_LAM : other->tag == Tag_Sup ? DUP_SUP : other->tag == Tag_Null ? DUP_NULL : NULL;
  }
  
  if (handler != NULL){
    runtime->steps ++;
    if (DEBUG) printf(BLUE "%d: HANDLE %s -> %s\n" RESET,  runtime->steps, tag_name(term->tag), tag_name(other->tag));
    handler(term, other);
    return 1;
  }
  return 0;
}



int stack_has(SearchStack* stack,Node* term){
  SearchStack* current = stack;
  while (current != NULL){
    if (current->node == term) return 1;
    current = current->next;
  }
  return 0;
}

void stack_push(SearchStack** stack, Node* term){
  SearchStack* tmp = (*stack);
  (*stack) = malloc(sizeof(SearchStack));
  (*stack)->node = term;
  (*stack)->next = tmp;
}


void stack_free(SearchStack** stack){
  while (*stack != NULL){
    SearchStack* tmp = *stack;
    *stack = (*stack)->next;
    free(tmp);
  }
  *stack = NULL;
}


int full_search = 0;
SearchStack* redex_seen = NULL; 


int search_redex(Node* term){

  printf("search_redex %s\n", tag_name(term->tag));

  if (term == NULL || term->s0 == NULL) return 0;
  Node* other = term->s0;

  if (handle_redex(term, other)){
    search_redex(term);
    return 1;
  }

  switch (term->tag){
    case Tag_Lam:
      search_redex(other);
      return 0;
    case Tag_Sup:
      search_redex(term->s0);
      search_redex(term->s1);
      return 0;
    case Tag_Dup: case Tag_Dup2:

      // if (full_search){
      //   if (stack_has(redex_seen, term->s0)) {
      //     printf("stack_has %p\n", term->s0);
      //     return 0;
      //   }
      //   stack_push(&redex_seen, term->s0);
      // }
      if (search_redex(other)) return search_redex(term);

      return 0;
    case Tag_App:
      if (search_redex(other)) return search_redex(term);
      // if (full_search){
      //   search_redex(term->s1);
      // }
      return 0;
    
    case Tag_Var: case Tag_Null: return 0;
    case Tag_Freed: error("Node is freed"); exit(1);
  }
}



int run(int Nsteps){
  fuel = Nsteps;
  search_redex(&(runtime->nodes[0]));
  return runtime->steps;
}





void load(int* data){

  struct sigaction sa;
  struct sigaction old_sa;
  sa.sa_handler = segfault_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGSEGV, &sa, &old_sa);
  
  segfault_occurred = 0;
  
  if (setjmp(segfault_jmp) != 0) {
    sigaction(SIGSEGV, &old_sa, NULL);
    error("SEGFAULT caught in C code\n");
  }

  if (runtime != NULL) error("new_runtime: runtime already exists\n");

  runtime = malloc(sizeof(Runtime));
  runtime->empty_index = 0;
  runtime->node_ctr = 0;
  runtime->free_list = NULL;
  runtime->steps = 0;

  int count = data[0];

  if (DEBUG) printf("LOAD: %d nodes\n", count);

  Node** nodes = malloc(sizeof(void*) * (count + 1));
  nodes[0] = NULL;
  
  for (int i = 0; i < count; i++) {
    int idx = i * 4 + 1;
    Node* node = new_node(data[idx], data[idx + 1], NULL, NULL);
    nodes[i + 1] = node;
    if (DEBUG >= 2) printf("  created [%d] tag=%s label=%d\n", i + 1, tag_name(data[idx]), data[idx + 1]);
  }
  
  for (int i = 0; i < count; i++) {
    int idx = i * 4 + 1;
    int s0_idx = data[idx + 2];
    int s1_idx = data[idx + 3];
    nodes[i + 1]->s0 = nodes[s0_idx];
    nodes[i + 1]->s1 = nodes[s1_idx];
    if (DEBUG >= 2) printf("  connected [%d] s0=%d s1=%d\n", i + 1, s0_idx, s1_idx);
  }

  free(nodes);
  
}

int* unload(){

  if (runtime == NULL) error("unload: runtime is NULL\n");

  int* result = serialize(&(runtime->nodes[0]));
  free(runtime);
  runtime = NULL;
  return result;
}
