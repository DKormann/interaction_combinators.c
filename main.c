
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>





int DEBUG = 1;


void set_debug(int debug){
  DEBUG = debug;
}


#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define RESET   "\x1b[0m"




void debug(char* content){
  if (DEBUG){
    printf(YELLOW "DEBUG: %s" RESET, content);
  }
}

void error(char* content){

  printf(RED "Error: %s\n" RESET, content);
  exit(1);
}


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
    error("remove on leaf error\n");
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

#define MAX_NODES 1000000

typedef struct Runtime{
  Node nodes[MAX_NODES];
  int empty_index;
  int node_ctr;
  Node* free_list;
  int steps;
} Runtime;

Runtime* runtime;



char* tag_name(int tag){
  if (tag == 10){
    return "FREED";
  }
  switch (tag){
    case Tag_App: return "App";
    case Tag_Lam: return "Lam";
    case Tag_Sup: return "Sup";
    case Tag_Dup: return "Dup";
    case Tag_Dup2: return "Dup2";
    case Tag_Null: return "Null";
    case Tag_Var: return "Var";
    case Tag_Freed: return "Freed";
  }
  return "UNK tag";
}


void check_node(Node* node);

Node* fresh_node(Tag tag, int label){
  Node* node = &runtime->nodes[runtime-> empty_index ++ ];
  if (runtime->empty_index >= MAX_NODES){
    printf("Error: MAX_NODES reached\n");
    exit(1);
  }
  runtime->node_ctr ++;
  node->tag = tag;
  node->label = label;
  node->s0 = NULL;
  node->s1 = NULL;
  return node;
}

Node* new_node(Tag tag, int label){
  Node* node = NULL;
  if (runtime->free_list != NULL){

    if (runtime->free_list->s0 == runtime->free_list){
      printf("free list is corrupted\n");
      exit(1);
    }
    node = runtime->free_list;
    runtime->free_list = node->s0;
    if (runtime->free_list == node){
      printf("free list is corrupted\n");
      exit(1);
    }
  }else{
    return fresh_node(tag, label);
  }
  runtime->node_ctr ++;
  node->tag = tag;
  node->label = label;
  node->s0 = NULL;
  node->s1 = NULL;
  printf("new node: %s %p\n", tag_name(tag), node);
  return node;
}



void free_node(Node* node){
  if (node->tag == Tag_Freed){
    printf("Error: Node %p is already freed\n", node);
    exit(1);
  }
  printf("free node %s %p\n", tag_name(node->tag), node);
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
  check_node(res);
  return res;
}


void print_tag(Node* node){
  printf(" <> %s %d\n", tag_name(node->tag), node->label);
}

void print_term(Node* node){
  walk_term(node, print_tag);
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
      check_null(node->s0, "Lam->s0");
      if (node->s1 != NULL && node->s1->tag != Tag_Var){
        printf(RED "Error : Lam %p ->s1->tag == %s %p instead of Var\n" RESET, node, tag_name(node->s1->tag), node->s1);
        exit(1);
      }
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


void check_tags(Node* node, Node* other, Tag tag, Tag other_tag){

  
  check_node(node);
  check_node(other);
  if (DEBUG){
    printf("%s -> %s\n", tag_name(tag), tag_name(other_tag));
  }
  if (node->tag != tag){
    printf("check Error: node->tag: %s != %s\n", tag_name(node->tag), tag_name(tag));
    exit(1);
  }
  if (other->tag != other_tag){
    printf("check Error: other->tag: %s != %s\n", tag_name(other->tag), tag_name(other_tag));
    exit(1);
  }
}



void erase(Node* node);

void just_move(Node* src, Node* dst){

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
  printf("move %s %p -> %p\n", tag_name(src->tag), src, dst);
  just_move(src,dst);
  free_node(src);
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
    case Tag_Freed: error("Node is freed");
  };
  free_node(node);
}



BST* taken;

void own(Node* owned){
  if (has_bst(taken, owned)){
    printf(RED "Node %s %p is already taken\n" RESET, tag_name(owned->tag), owned);
    exit(1);
  }
  insert_bst(taken, owned);
}

void _deepcheck(Node* term, BST* visited){
  // printf("check node %s %p -> %p\n", tag_name(term->tag), term, term->s0);
  check_node(term);
  switch (term->tag){
    case Tag_App:
    case Tag_Sup:
      own(term->s0);
      own(term->s1);
      _deepcheck(term->s0, visited);
      _deepcheck(term->s1, visited);
      break;
    case Tag_Lam:
      own(term->s0);
      _deepcheck(term->s0, visited);
      break;
    case Tag_Dup:
      own(term->s0);
    case Tag_Dup2:
      if (has_bst(visited, term->s0)){
        return;
      }
      insert_bst(visited, term->s0);
      _deepcheck(term->s0, visited);
      break;
    case Tag_Null:
    case Tag_Var: break;
    case Tag_Freed: printf(RED "Node %p is freed\n", term); exit(1);
  };
}


void deepcheck(){
  printf("deepcheck\n");
  BST* visited = new_bst();
  taken = new_bst();
  _deepcheck(&runtime->nodes[0], visited);
  free_bst(visited);
  free_bst(taken);
  printf("deepcheck OK\n");
}



int APP_LAM(Node* App, Node* Lam){

  Node* arg = App->s1;
  Node* var = Lam->s1;
  Node* body = Lam->s0;

  // printf("APP_LAM app lam arg var body %p %p %p %p %p\n", App, Lam, arg, var, body);
  
  if (var != NULL){
    move(arg, var);
    move(body, App);
    if (Lam->s1 != NULL && Lam->s1->s0 == Lam){
      printf("cannot free lam %p it has a var\n", Lam);
      exit(1);
    }
  }else{
    move(body, App);
    erase(arg);
  }

  free_node(Lam);
  return 1;
}

int APP_SUP(Node* App, Node* Sup){
  check_tags(App, Sup, Tag_App, Tag_Sup);
  printf("app arg:%s %p\n", tag_name(App->s1->tag), App->s1);
  // Node** dups = mk_dup(App->s1, Sup->label);

  Node** dups = malloc(sizeof(Node*) * 2);
  dups[0] = fresh_node(Tag_Dup, Sup->label);
  dups[1] = fresh_node(Tag_Dup2, Sup->label);
  dups[0]->s0 = App->s1;
  dups[1]->s0 = App->s1;
  dups[0]->s1 = dups[1];
  dups[1]->s1 = dups[0];


  printf("APP_SUP dups[0] %p\n", dups[0]);
  printf("APP_SUP dups[1] %p\n", dups[1]);
  
  move(sup(app(Sup->s0, dups[0]), app(Sup->s1, dups[1]), Sup->label), App);

  free(dups);

  return 1;
}


int APP_NULL(Node* App, Node* Null){
  check_tags(App, Null, Tag_App, Tag_Null);
  erase(App->s1);
  move(Null, App);
  return 1;
}


int DUP_LAM(Node* dup, Node* Lam){



  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;

  printf("da:%p db:%p lam:%p var:%p\n",  da, db, Lam, Lam->s1);



  int label = da == NULL ? db->label : da->label;
  
  Node** dbody = mk_dup(Lam->s0, label);

  printf("dups:%p %p\n", dbody[0], dbody[1]);

  Node* funa = new_node(Tag_Lam,0);
  Node* funb = new_node(Tag_Lam,0);

  printf("funa, funb:%p %p\n", funa, funb);
  funa->s0 = dbody[0];
  funb->s0 = dbody[1];

  
  free(dbody);
  if (Lam->s1 != NULL){
    debug("DUP_LAM Lam->s1 != NULL\n");
    funa->s1 = new_node(Tag_Var, 0);
    funa->s1->s0 = funa;
    funb->s1 = new_node(Tag_Var, 0);
    funb->s1->s0 = funb;
    move(sup(funa->s1, funb->s1, label), Lam->s1);
  }else{
    debug("DUP_LAM Lam->s1 == NULL\n");
  }
  free_node(Lam);

  move(funa, da);
  printf("DUP_LAM funb->db\n");
  move(funb, db);

  printf("DUP_LAM DONE\n");



  return 1;
}

int DUP_SUP(Node* dup, Node* Sup){

  check_tags(dup, Sup, dup->tag == Tag_Dup ? Tag_Dup : Tag_Dup2, Tag_Sup);
  Node* da = dup->tag == Tag_Dup ? dup : dup->s1;
  Node* db = dup->tag == Tag_Dup2 ? dup : dup->s1;
  int label = da == NULL ? db->label : da->label;
  if (Sup->label == label){

    if (Sup->s0 == db || Sup->s1 == db){
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

int fuel = 0;


int handle_redex(Node* term, Node* other){
  if (fuel < runtime->steps){
    return 0;
  }

  int (*handler)(Node*, Node*) = NULL;

  switch (term->tag){
    case Tag_App: {
      switch (other->tag){
        case Tag_Lam: {handler = APP_LAM; break;}
        case Tag_Sup: {handler = APP_SUP; break;}
        case Tag_Null: {handler = APP_NULL; break;}
        default: break;
      };
      break;
    };
    case Tag_Dup: case Tag_Dup2:{
      switch (other->tag){
        case Tag_Lam: {handler = DUP_LAM; break;}
        case Tag_Sup: {handler = DUP_SUP; break;}
        case Tag_Null: {handler = DUP_NULL; break;}
        default: break;
      };
      break;
    }
    default: break;
  };
  if (handler != NULL){

    runtime->steps ++;


    if (DEBUG) printf(BLUE "%d: HANDLE %s -> %s\n" RESET, runtime->steps, tag_name(term->tag), tag_name(other->tag));

    handler(term, other);


    deepcheck();


    return 1;

  }
  return 0;
}



BST* searched_lams;


int search_redex(Node* term){
  

  if (term == NULL){
    return 0;
  }
  Node* other = term->s0;

  if (other == NULL){
    return 0;
  }

  // printf("pre search check\n");
  // check_node(term);
  // check_node(other);
  // printf("OK\n");

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
      if (search_redex(other)) return search_redex(term);
      return 0;
    case Tag_Var:
    case Tag_Null: return 0;
    case Tag_Freed: error("Node is freed"); exit(1);
  }
}


int run(int Nsteps){

  fuel = Nsteps;


  if (DEBUG) printf("RUN %d\n", Nsteps);
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
  
  deepcheck();
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
