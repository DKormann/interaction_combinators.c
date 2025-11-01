

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


BST* taken;

void own(Node* owned){
  if (has_bst(taken, owned)){
    printf(RED "Node %s %p is already taken\n" RESET, tag_name(owned->tag), owned);
    exit(1);
  }
  insert_bst(taken, owned);
}

void _deepcheck(Node* term, BST* visited){
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
