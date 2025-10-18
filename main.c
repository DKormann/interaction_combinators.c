
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>



typedef enum Tag{
  Tag_App,
  Tag_Lam,
  Tag_Sup,
  Tag_Dup,
  Tag_Dup2,
  Tag_Null,
  Tag_Var,
}Tag;

typedef struct Node{
  Tag tag;
  int label;
  struct Node* s0;
  struct Node* s1;
}Node;



typedef struct Queue{
  Node* node;
  int s0;
  int s1;
  struct Queue* next;
} Queue;

static jmp_buf segfault_jmp;
static volatile sig_atomic_t segfault_occurred = 0;

void segfault_handler(int sig) {
  segfault_occurred = 1;
  longjmp(segfault_jmp, 1);
}


char* tag_name(int tag){
  switch (tag){
    case Tag_App: return "App";
    case Tag_Lam: return "Lam";
    case Tag_Sup: return "Sup";
    case Tag_Dup: return "Dup";
    case Tag_Dup2: return "Dup2";
    case Tag_Null: return "Null";
    case Tag_Var: return "Var";
  }
  return "UNK tag";
}

int enqueue(Queue* queue, Node* node, int * ctr){
  if (node == NULL){
    return 0;
  }

  
  int n = 0;
  
  while (1){
    if (queue->node == node){
      return n + 1;
    }
    n ++ ;
    if (queue->next == NULL){
      break;
    }
    queue = queue->next;
  }
  
  Queue* new_node = malloc(sizeof(Queue));
  new_node->node = node;
  new_node->next = NULL;
  queue->next = new_node;
  (*ctr)++;

  return n + 1;
}


int* serialize(Node* node){

  Queue* queue = malloc(sizeof(Queue));
  queue->node = node;
  queue->next = NULL;
  Queue* current = queue;
  int ctr = 1;

  while (current != NULL){
    Node* node = current->node;
    current -> s0 = enqueue(queue, node->s0, &ctr);
    current -> s1 = enqueue(queue, node->s1, &ctr);
    current = current->next;
  }



  int* result = malloc(sizeof(int) * (ctr + 1) * 4);
  current = queue;

  result[0] = ctr;
  ctr = 1;


  while (1){

    result[ctr] = current->node->tag;
    result[ctr + 1] = current->node->label;
    result[ctr + 2] = current->s0;
    result[ctr + 3] = current->s1;
    ctr += 4;
    Queue* prev = current;
    current = current->next;
    if (current == NULL){
      break;
    }
    free(prev);
  }

  return result;
}



Node* new_node(Tag tag, int label){
  Node* node = malloc(sizeof(Node));
  node->tag = tag;
  node->label = label;
  node->s0 = NULL;
  node->s1 = NULL;
  return node;
}

Node** dup(Node* target, int label){
  Node* dup1 = new_node(Tag_Dup, label);
  Node* dup2 = new_node(Tag_Dup2, label);
  dup1->s1 = dup2;
  dup2->s1 = dup1;
  dup1->s0 = target;
  dup2->s0 = target;
  Node**res = malloc(sizeof(Node*) * 2);
  res[0] = dup1;
  res[1] = dup2;
  return res;
}

Node* fun(Node* body){
  Node* var = new_node(Tag_Var, 0);
  Node* res = new_node(Tag_Lam, 0);
  res->s0 = body;
  res->s1 = var;
  var->s0 = res;
  return res;
}

Node* sup(Node* a, Node* b, int label){
  Node* res = new_node(Tag_Sup, label);
  res->s0 = a;
  res->s1 = b;
  return res;
}

Node* app(Node* f, Node* x){
  Node* res = new_node(Tag_App, 0);
  res->s0 = f;
  res->s1 = x;
  return res;
}


void move(Node* src, Node* dst){

  dst->tag = src->tag;
  dst->s0 = src->s0;
  dst->s1 = src->s1;

  dst->label = src->label;
  if (src->tag == Tag_Var){
    if (src->s0->tag != Tag_Lam){
      printf("Error: Invalid tag for lam in move\n");
      exit(1);
    }
    src->s0->s1 = dst;
  }
  if (src->tag == Tag_Lam && src->s1 != NULL){
    src->s1->s0 = dst;
  }
  if (src->tag == Tag_Dup || src->tag == Tag_Dup2){
    dst->s1->s1 = dst;
  }
}



int APP_LAM(Node* App, Node* Lam){
  if (Lam->s1 != NULL){
    move(App->s1, Lam->s1);
  }else{
    free(App->s1);
  }
  move(Lam->s0, App);
  free(Lam);
  return 1;
}

int APP_SUP(Node* App, Node* Sup){
  Node** dups = dup(App->s1, Sup->label);
  move(sup(app(Sup->s0, dups[0]), app(Sup->s1, dups[1]), Sup->label), App);
  return 1;
}

int DUP_LAM(Node* da, Node* db, Node* Lam){
  Node** dbody = dup(Lam->s0, da->label);
  Node* funa = fun(dbody[0]);
  Node* funb = fun(dbody[1]);
  free(dbody);
  if (Lam->s1 != NULL){
    move(sup(funa->s1, funb->s1, da->label), Lam->s1);
  }
  move(funa, da);
  move(funb, db);
  return 1;
}

int DUP_SUP(Node* da, Node* db, Node* Sup){
  if (Sup->label == da->label){

    if (Sup->s0 == db || Sup->s1 == db){
      move(Sup->s1, db);
      move(Sup->s0, da);
    }else{
      move(Sup->s0, da);
      move(Sup->s1, db);

    }
  } else {
    Node** dup1 = dup(Sup->s0, da->label);
    Node** dup2 = dup(Sup->s1, da->label);
    move(sup(dup1[0], dup2[0], Sup->label), da);
    move(sup(dup1[1], dup2[1], Sup->label), db);
    free(dup1);
    free(dup2);
  }
  free(Sup);
  return 1;
}




int step(Node* term){
  if (term == NULL || term->tag == Tag_Null || term->tag == Tag_Var){
    return 0;
  }
  Node* other = term->s0;
  if (other == NULL){
    return 0;
  }
  switch (term->tag){
    case Tag_App:
      switch (other->tag){
        case Tag_Lam: return APP_LAM(term, other);
        case Tag_Dup:
        case Tag_Dup2: return step(other);
        case Tag_Sup:{ return APP_SUP(term, other);
        }
        default: break;
      }
      break;
    case Tag_Sup: return step(other) || step(term->s1);      
    case Tag_Lam: return step(other);

    case Tag_Dup: case Tag_Dup2:{
      Node* da = term->tag == Tag_Dup ? term : term->s1;
      Node* db = da->s1;
      switch (other->tag){
        case Tag_Lam:{
          return DUP_LAM(da, db, other);
        }
        case Tag_Sup:{
          return DUP_SUP(da, db, other);
        }
        case Tag_Null:{
          move(other, da);
          move(new_node(Tag_Null, 0), db);
          return 1;
        }
        case Tag_App:{
          return step(other);
        }
        case Tag_Var:{
          return 0;
        }
        default: break;
      }
      break;
    }
    default:break;
  }
  printf("TODO: handle %s -> %s\n", tag_name(term->tag), tag_name(other->tag));
  return 0;
}

void run(Node* node, int steps){
  while (steps > 0 && step(node)){
    steps--;
  }
}

Node* deserialize(int* data) {
  int count = data[0];
  if (count == 0) return NULL;
  
  Node** nodes = malloc(sizeof(Node*) * (count + 1));
  nodes[0] = NULL;
  
  for (int i = 0; i < count; i++) {
    int idx = i * 4 + 1;
    Node* node = malloc(sizeof(Node));
    node->tag = data[idx];
    node->label = data[idx + 1];
    node->s0 = NULL;
    node->s1 = NULL;
    nodes[i + 1] = node;
  }
  
  for (int i = 0; i < count; i++) {
    int idx = i * 4 + 1;
    int s0_idx = data[idx + 2];
    int s1_idx = data[idx + 3];
    nodes[i + 1]->s0 = nodes[s0_idx];
    nodes[i + 1]->s1 = nodes[s1_idx];
  }
  
  Node* root = nodes[1];
  free(nodes);
  return root;
}

int* work(int* graph_data, int steps){
  // Install segfault handler
  struct sigaction sa;
  struct sigaction old_sa;
  sa.sa_handler = segfault_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGSEGV, &sa, &old_sa);
  
  segfault_occurred = 0;
  
  // Set up recovery point
  if (setjmp(segfault_jmp) != 0) {
    // Segfault occurred - restore old handler and return error
    sigaction(SIGSEGV, &old_sa, NULL);
    fprintf(stderr, "SEGFAULT caught in C code\n");
    // Return a special error value: [-1] indicates error
    int* error_result = malloc(sizeof(int));
    error_result[0] = -1;
    return error_result;
  }
  
  // Normal execution
  Node* node = deserialize(graph_data);
  run(node, steps);
  int* fmt = serialize(node);
  
  // Restore old handler
  sigaction(SIGSEGV, &old_sa, NULL);
  
  return fmt;
}

