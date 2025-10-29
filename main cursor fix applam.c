
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>


typedef struct BST{
  int is_leaf;
  unsigned long value;
  struct BST* left;
  struct BST* right;
} BST;

typedef struct BSTLeaf{
  int is_leaf;
  unsigned long value;
} BSTLeaf;



unsigned long hash_ptr(void* ptr){
  return ((unsigned long)ptr) * 83794261827 + 3489;
}



void _insert_bst(BST* tree, unsigned long value){
  if (tree->value == value){
    return;
  }
  int tree_bigger = tree->value > value;
  BST* next = tree_bigger ? tree->left : tree->right;
  if (next->is_leaf){
    BST* node = malloc(sizeof(BST));
    BST* leaf = malloc(sizeof(BSTLeaf));
    leaf->is_leaf = 1;
    leaf->value = value;
    node->is_leaf = 0;
    int next_bigger = next->value > value;
    node->value = next_bigger ? value : next->value;
    node->left = next_bigger ? leaf : next;
    node->right = next_bigger ? next : leaf;
    if (tree_bigger){
      tree->left = node;
    }else{
      tree->right = node;
    }
  }else{
    _insert_bst(next, value);
  }
}

void insert_bst(BST* tree, void* value){
  _insert_bst(tree, hash_ptr (value));
}

void _remove_bst(BST** tree, unsigned long value){
  if ((*tree)->is_leaf){
    printf("remove on leaf error\n");
    exit(1);
  }
  int tree_be = (*tree)->value >=  value;
  BST* next = tree_be? (*tree)->left : (*tree)->right;
  if (next->is_leaf){
    if (next->value == value){
      free(next);
      BST* other = tree_be ? (*tree)->right : (*tree)->left;
      BST*old = *tree;
      (*tree) = other;
      free(old);
      return;
    }
  }else{
    if (tree_be){ _remove_bst(&((*tree)->left), value);
    }else{ _remove_bst(&((*tree)->right), value);}
  }
}

void remove_bst(BST** tree, void* value){
  _remove_bst(tree, hash_ptr(value));
}

int _has_bst(BST* tree, unsigned long value){
  if (tree->value == value){
    return 1;
  }
  if (tree->is_leaf){
    return 0;
  }
  return _has_bst(tree->value > value ? tree->left : tree->right, value);
}

int has_bst(BST* tree, void* value){
  return _has_bst(tree, hash_ptr(value));
}

void _print_bst(BST* tree, int indent){

  if (tree->is_leaf){
    printf("%lu\n", tree->value);
  }else{
    _print_bst(tree->left, indent + 1);
    _print_bst(tree->right, indent + 1);
  }
}

void print_bst(BST* tree){
  printf("BST: ");
  _print_bst(tree->right->right, 0);
  printf("\n");
}

void free_bst(BST* tree){
  if (tree->is_leaf){
    free(tree);
  }else{
    free_bst(tree->left);
    free_bst(tree->right);
    free(tree);
  }
}

BST* new_bst(){
  BST* tree = malloc(sizeof(BST));
  tree->is_leaf = 0;
  tree->value = 0;
  tree->left = malloc(sizeof(BSTLeaf));
  tree->left->is_leaf = 1;
  tree->left->value = 0;
  tree->right = malloc(sizeof(BSTLeaf));
  tree->right->is_leaf = 1;
  tree->right->value = 0;
  return tree;
};



int DEBUG = 1;


void set_debug(int debug){
  DEBUG = debug;
}

void debug(char* content){
  if (DEBUG){

    printf("%s", content);
  }
}

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

#define MAX_NODES 1000000

typedef struct Runtime{
  Node nodes[MAX_NODES];
  int empty_index;
  int node_ctr;
  Node* free_list;
  int steps;
} Runtime;

Runtime* runtime;


Node* new_node(Tag tag, int label){
  Node* node;
  if (runtime->free_list != NULL){
    node = runtime->free_list;
    runtime->free_list = node->s0;
  }else{
    node = &runtime->nodes[runtime-> empty_index ++ ];
  }
  runtime->node_ctr ++;
  node->tag = tag;
  node->label = label;
  node->s0 = NULL;
  node->s1 = NULL;
  return node;
}

void free_node(Node* node){
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

static jmp_buf segfault_jmp;
static volatile sig_atomic_t segfault_occurred = 0;

void segfault_handler(int sig) {
  segfault_occurred = 1;
  longjmp(segfault_jmp, 1);
}


void walk_cached(Node* node, void callback(Node*), BST* visited){
  if (node == NULL){
    return;
  }
  
  callback(node);
  switch (node->tag){
    case Tag_App:
    case Tag_Sup:
      walk_cached(node->s0, callback, visited);
      walk_cached(node->s1, callback, visited);
      break;
    case Tag_Lam:
      walk_cached(node->s0, callback, visited);
      break;
    case Tag_Dup:
    case Tag_Dup2:
      if (node->s0 != NULL){
        if (!has_bst(visited, node->s0)){
          insert_bst(visited, node->s0);
          walk_cached(node->s0, callback, visited);
        }else{
          remove_bst(&visited, node->s0);
        }
      }
    default: break;
  }
}

void walk_term(Node* node, void callback(Node*)){
  BST* visited = new_bst();
  walk_cached(node, callback, visited);
  free_bst(visited);
}

/* SERIALIZATION */


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

int _enqueue(S_Queue* queue, Node* node, int * ctr){
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

  if (DEBUG) printf("SERIALIZE: %d nodes\n", result[0]);

  while (1){

    result[ctr] = current->node->tag;
    result[ctr + 1] = current->node->label;
    result[ctr + 2] = current->s0;
    result[ctr + 3] = current->s1;
    if (DEBUG) printf("  [%d] tag=%s label=%d s0=%d s1=%d\n", (ctr-1)/4 + 1, tag_name(current->node->tag), current->node->label, current->s0, current->s1);
    ctr += 4;
    S_Queue* prev = current;
    current = current->next;
    if (current == NULL){
      break;
    }
    free(prev);
  }

  return result;
}



Node** mk_dup(Node* target, int label){
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

Node* sup(Node* a, Node* b, int label){
  if (a == NULL){
    printf("sup a is NULL\n");
    exit(1);
  }
  if (b == NULL){
    printf("sup b is NULL\n");
    exit(1);
  }
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


void print_tag(Node* node){
  printf(" <> %s %d\n", tag_name(node->tag), node->label);
}

void print_term(Node* node){
  walk_term(node, print_tag);
}




void erase(Node* node);

void just_move(Node* src, Node* dst){

  if (dst == NULL){
    erase(src);
    return;
  }


  dst->tag = src->tag;
  dst->s0 = src->s0;
  dst->s1 = src->s1;

  dst->label = src->label;
  if (src->tag == Tag_Var){
    if (src->s0->tag != Tag_Lam){
      printf("Error: Var points to %s instead of Lam\n", tag_name(src->s0->tag));
      printf("  Var at: %p\n", (void*)src);
      printf("  Var->s0 at: %p (tag: %s)\n", (void*)src->s0, tag_name(src->s0->tag));
      printf("  Destination at: %p\n", (void*)dst);
      exit(1);
    }
    src->s0->s1 = dst;
  }
  if (src->tag == Tag_Lam && src->s1 != NULL){
    if (src->s1->tag != Tag_Var){
      printf("Error: Invalid tag for var in move\n");
      exit(1);
    }
    src->s1->s0 = dst;
  }
  if (src->tag == Tag_Dup || src->tag == Tag_Dup2){
    if (dst->s1 != NULL){
      dst->s1->s1 = dst;
    }
  }

}

void move(Node* src, Node* dst){
  if (dst == NULL){
    erase(src);
    return;
  }
  just_move(src,dst);

}


typedef struct DupStack{
  int label;
  int is_dup2;
  struct DupStack* next;
} DupStack;


void print_stack(DupStack* stack){
  printf("[");
  while (stack != NULL){
    printf("%d ", stack->label);
    stack = stack->next;
  }
  printf("]\n");
}

void erase(Node* node){
  switch (node->tag){
    case Tag_App:
    case Tag_Sup: erase(node->s1);
    case Tag_Lam:
      erase(node->s0);
      break;
    case Tag_Var:
      node->s0->s1 = NULL;
      break;
    case Tag_Dup:
    case Tag_Dup2:{
      if (node->s1 == NULL){
        erase(node->s0);
      }else{
        node->s1->s1 = NULL;
      }
      
      break;
    }
    case Tag_Null: break;
  };
  free_node(node);
}

void check_tags(Node* node, Node* other, Tag tag, Tag other_tag){
  if (DEBUG){
    printf("%s -> %s\n", tag_name(node->tag), tag_name(other->tag));
  }
  if (node->tag != tag){
    printf("Error: node->tag: %s != %s\n", tag_name(node->tag), tag_name(tag));
    exit(1);
  }
  if (other->tag != other_tag){
    printf("Error: other->tag: %s != %s\n", tag_name(other->tag), tag_name(other_tag));
    exit(1);
  }
}

int APP_LAM(Node* App, Node* Lam){
  check_tags(App, Lam, Tag_App, Tag_Lam);
  // Save the argument before we start modifying
  Node* arg = App->s1;
  // Handle the argument AFTER we've replaced App
  Node* body = Lam->s0;
  Node* var = Lam->s1;
  
  // Atomically replace App with the body - this is done with just_move
  // which copies all fields at once, so App is never in an invalid state
  just_move(body, App);
  
  // Now handle the argument
  if (var != NULL){
    move(arg, var);
  }else{
    erase(arg);
  }
  
  free_node(Lam);
  return 1;
}

int APP_SUP(Node* App, Node* Sup){
  check_tags(App, Sup, Tag_App, Tag_Sup);
  Node** dups = mk_dup(App->s1, Sup->label);
  move(sup(app(Sup->s0, dups[0]), app(Sup->s1, dups[1]), Sup->label), App);
  return 1;
}


int APP_NULL(Node* App, Node* Null){
  check_tags(App, Null, Tag_App, Tag_Null);
  // Save the argument pointer first
  Node* arg = App->s1;
  // Atomically replace App with Null
  just_move(Null, App);
  // Now erase the saved argument
  erase(arg);
  return 1;
}


int DUP_LAM(Node* dup, Node* Lam){
  check_tags(dup, Lam, dup->tag == Tag_Dup ? Tag_Dup : Tag_Dup2, Tag_Lam);
  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;
  int label = da == NULL ? db->label : da->label;
  Node** dbody = mk_dup(Lam->s0, label);
  Node* funa = new_node(Tag_Lam,0);
  Node* funb = new_node(Tag_Lam,0);
  funa->s0 = dbody[0];
  funb->s0 = dbody[1];

  
  free(dbody);
  if (Lam->s1 != NULL){
    funa->s1 = new_node(Tag_Var, 0);
    funa->s1->s0 = funa;
    funb->s1 = new_node(Tag_Var, 0);
    funb->s1->s0 = funb;
    move(sup(funa->s1, funb->s1, label), Lam->s1);

  }
  free_node(Lam);
  move(funa, da);
  move(funb, db);
  return 1;
}

int DUP_SUP(Node* dup, Node* Sup){
  check_tags(dup, Sup, dup->tag == Tag_Dup ? Tag_Dup : Tag_Dup2, Tag_Sup);
  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;
  int label = da == NULL ? db->label : da->label;
  if (Sup->label == label){
    debug("DUP_SUP_SAME_LABEL\n");

    if (Sup->s0 == db || Sup->s1 == db){
      debug("DUP_SUP_SAME_LABEL_SAME_DB\n");
      move(Sup->s1, db);
      move(Sup->s0, da);
    }else{

      if (da != NULL){
        move(Sup->s0, da);
      }else{
        erase(Sup->s0);
      }
      if (db != NULL){
        move(Sup->s1, db);
      }else{
        erase(Sup->s1);
      }
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
  check_tags(dup, Null, dup->tag == Tag_Dup ? Tag_Dup : Tag_Dup2, Tag_Null);
  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;
  just_move(Null, da);
  move(Null, db);
  return 1;
}




int full_redex_search = 1;
BST* visited;

int step(Node* term){

  if (term == NULL){
    return 0;
  }

  Node* other = term->s0;
  if (other == NULL){
    return 0;
  }
  if (DEBUG){ printf("step %s -> %s\n", tag_name(term->tag), tag_name(other->tag));}

  switch (term->tag){
    case Tag_App:
      switch (other->tag){
        case Tag_Lam: return APP_LAM(term, other);
        case Tag_Sup: return APP_SUP(term, other);
        case Tag_Dup:
        case Tag_Dup2:
        case Tag_App:
          if (step(other)){
            return 1;
          }
          if (full_redex_search){
            return step(term->s1);
          }
          return 0;
        case Tag_Var: return step(term->s1);
        case Tag_Null:
          erase(term->s1);
          move(other, term);
      }
      break;
    case Tag_Sup: return step(other) || step(term->s1);      
    case Tag_Lam: return step(other);

    case Tag_Dup: case Tag_Dup2:{
      if (full_redex_search){
        if (has_bst(visited, other)){
          return 0;
        }
        insert_bst(visited, other);
      }
      Node* da = term->tag == Tag_Dup ? term : term->s1;
      Node* db = term->tag == Tag_Dup2 ? term : term->s1;
      switch (other->tag){
        case Tag_Lam:{
          return DUP_LAM(term, other);
        }
        case Tag_Sup:{
          return DUP_SUP(term, other);
        }
        case Tag_Null:{
          just_move(other, da);
          move(other, db);          
          return 1;
        }
        case Tag_App:{
          return step(other);
        }
        case Tag_Var:{
          return 0;
        }
        case Tag_Dup:
        case Tag_Dup2:{
          return step(other);
        }
      }
      break;
    }
    case Tag_Var:
    case Tag_Null:
      return 0;
  }
  printf("TODO: handle %s -> %s\n", tag_name(term->tag), tag_name(other->tag));
  return 0;
}




int handle_redex(Node* term, Node* other){
  switch (term->tag){
    case Tag_App:
      if (term->s1 == NULL){
        printf("ERROR: App node has NULL s1 (argument)\n");
        printf("  App at memory: %p\n", (void*)term);
        printf("  App->s0 (function): %p (tag: %s)\n", (void*)term->s0, term->s0 ? tag_name(term->s0->tag) : "NULL");
        printf("  App->s1 (argument): NULL\n");
        printf("  Other (s0): %p (tag: %s)\n", (void*)other, tag_name(other->tag));
        printf("  This is likely an intermediate state from a reduction rule\n");
        printf("  Not attempting reduction on incomplete node\n");
        return 0;  // Don't reduce, skip this node
      }
      switch (other->tag){
        case Tag_Lam: return APP_LAM(term, other);
        case Tag_Sup: return APP_SUP(term, other);
        case Tag_Null: return APP_NULL(term, other);
        default: return 0;
      }
    case Tag_Dup: case Tag_Dup2:
      switch (other->tag){
        case Tag_Lam: return DUP_LAM(term, other);
        case Tag_Sup: return DUP_SUP(term, other);
        case Tag_Null: return DUP_NULL(term, other);
        default: return 0;
      }
    default: return 0;
  }
}


BST* searched_lams;


// return indicates whether term changed.
int search_redex(Node* term){
  // printf("search_redex %s -> %s\n", tag_name(term->tag), tag_name(term->s0->tag));
  if (term == NULL){
    return 0;
  }
  Node* other = term->s0;

  if (other == NULL){
    return 0;
  }

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
    case Tag_Dup: case Tag_Dup2: case Tag_App:
      if (search_redex(other)){
        return search_redex(term);
      }
      // if (full_redex_search && term->tag == Tag_App){
        // return search_redex(term->s1);
      // }
      return 0;
    case Tag_Var:
    case Tag_Null:
      return 0;
  }
}






int run(int Nsteps){
  int steps = 0;
  
  Node* term = &(runtime->nodes[0]);
  full_redex_search = 0;
  searched_lams = new_bst();
  search_redex(term);
  free_bst(searched_lams);
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
    fprintf(stderr, "SEGFAULT caught in C code\n");
    exit(1);
  }

  if (runtime != NULL){
    printf("new_runtime: runtime already exists\n");
    exit(1);
  }
  runtime = malloc(sizeof(Runtime));
  runtime->empty_index = 0;
  runtime->node_ctr = 0;
  runtime->free_list = NULL;
  runtime->steps = 0;

  int count = data[0];
  if (count == 0) {
    printf("deserialize: count is 0\n");
    exit(1);
  }

  if (DEBUG) printf("LOAD: %d nodes\n", count);

  Node** nodes = malloc(sizeof(void*) * (count + 1));
  nodes[0] = NULL;
  
  for (int i = 0; i < count; i++) {
    int idx = i * 4 + 1;
    Node* node = new_node(data[idx], data[idx + 1]);
    nodes[i + 1] = node;
    if (DEBUG) printf("  created [%d] tag=%s label=%d\n", i + 1, tag_name(data[idx]), data[idx + 1]);
  }
  
  for (int i = 0; i < count; i++) {
    int idx = i * 4 + 1;
    int s0_idx = data[idx + 2];
    int s1_idx = data[idx + 3];
    nodes[i + 1]->s0 = nodes[s0_idx];
    nodes[i + 1]->s1 = nodes[s1_idx];
    if (DEBUG) printf("  connected [%d] s0=%d s1=%d\n", i + 1, s0_idx, s1_idx);
  }
  
  Node* root = nodes[1];
  if (root != &(runtime->nodes[0])){
    printf("deserialize: root is not the first node\n");
  }
  free(nodes);

}

int* unload(){

  if (runtime == NULL){
    printf("unload: runtime is NULL\n");
    exit(1);
  }

  Node* node = &(runtime->nodes[0]);

  int* result = serialize(node);
  free(runtime);
  runtime = NULL;
  return result;
}
