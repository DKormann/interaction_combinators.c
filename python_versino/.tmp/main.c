
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



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



Node* setup(void){
Node* x0 = malloc(sizeof(Node));
Node* x1 = malloc(sizeof(Node));
Node* x2 = malloc(sizeof(Node));
Node* x3 = malloc(sizeof(Node));
Node* x4 = malloc(sizeof(Node));
x4->tag = Tag_Var;
x4->label = 0;
x4->s0 = x1;
Node* x5 = malloc(sizeof(Node));
x5->tag = Tag_Dup2;
x5->label = 72;
x5->s0 = x4;
x5->s1 = x3;
x3->tag = Tag_Dup;
x3->label = 72;
x3->s0 = x4;
x3->s1 = x5;
Node* x6 = malloc(sizeof(Node));
Node* x7 = malloc(sizeof(Node));
x7->tag = Tag_Null;
x7->label = 0;
x6->tag = Tag_App;
x6->label = 0;
x6->s0 = x5;
x6->s1 = x7;
x2->tag = Tag_Sup;
x2->label = 71;
x2->s0 = x3;
x2->s1 = x6;
x1->tag = Tag_Lam;
x1->label = 0;
x1->s0 = x2;
x1->s1 = x4;
Node* x8 = malloc(sizeof(Node));
Node* x9 = malloc(sizeof(Node));
x9->tag = Tag_Var;
x9->label = 0;
x9->s0 = x8;
x8->tag = Tag_Lam;
x8->label = 0;
x8->s0 = x9;
x8->s1 = x9;
x0->tag = Tag_App;
x0->label = 0;
x0->s0 = x1;
x0->s1 = x8;

return x0;
}



typedef struct Queue{
  Node* node;
  int s0;
  int s1;
  struct Queue* next;
} Queue;


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


int* format(Node* node){

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

    // printf("current: %s %d %d %d\n", tag_name(current->node->tag), current->node->label, current->s0, current->s1);

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


void move(Node* src, Node* dst){
  // printf("move: %s -> %s\n", tag_name(src->tag), tag_name(dst->tag));
  dst->tag = src->tag;
  dst->s0 = src->s0;
  dst->s1 = src->s1;
  dst->label = src->label;
  if (src->tag == Tag_Var){
    if (src->s0->tag != Tag_Lam){
      printf("Error: Invalid tag for lam in move\n");
      exit(1);
    }
    // printf("moving lam to var\n");
    src->s0->s1 = dst;
  }
  if (src->tag == Tag_Lam){
    // printf("moving var to lam\n");
    dst->s1->s0 = dst;
  }
  if (src->tag == Tag_Dup || src->tag == Tag_Dup2){
    dst->s1->s1 = dst;
  }
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



int step(Node* term){
  if (term == NULL || term->tag == Tag_Null || term->tag == Tag_Var){
    return 0;
  }
  Node* other = term->s0;
  if (other == NULL){
    return 0;
  }
  printf("step: %s -> %s\n", tag_name(term->tag), tag_name(other->tag));
  switch (term->tag){
    case Tag_App:
      switch (other->tag){
        case Tag_Lam:
          if (other->s1 != NULL){
            move(term->s1, other->s1);
          }
          move(other->s0, term);
          free(other);
          return 1;
        default: break;
      }
      break;
    case Tag_Sup: return step(other) || step(term->s1);      
    case Tag_Lam: return step(other);

    case Tag_Dup:
    case Tag_Dup2:{
      Node* da = term->tag == Tag_Dup ? term : term->s1;
      Node* db = da->s1;
      switch (other->tag){
        case Tag_Lam:{
          Node** dbody = dup(other->s0, term->label);
          Node* ba = dbody[0];
          Node* bb = dbody[1];
          free(dbody);
          Node* funa = fun(ba);
          Node* funb = fun(bb);
          if (other->s1 != NULL){
            move(sup(funa->s1, funb->s1, term->label), other->s1);
          }
          move(funa, da);
          move(funb, db);
          return 1;
        }

        case Tag_Sup:{
          if (other->label == da->label){
            move(other->s0, da);
            move(other->s1, db);
            free(other);
            return 1;
          } else {
            Node** dup1 = dup(other->s0, da->label);
            Node** dup2 = dup(other->s1, da->label);
            move(sup(dup1[0], dup2[0], other->label), da);
            move(sup(dup1[1], dup2[1], other->label), db);
            free(dup1);
            free(dup2);
            free(other);
            return 1;
          }
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


int* work(int steps){

  Node* node = setup();

  run(node, steps);

  int* fmt = format(node);
  return fmt;
}

