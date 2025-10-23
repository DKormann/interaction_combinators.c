#include <stdlib.h>
#include <stdio.h>

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






/*
 * this deepwalk tries to pair dup - sup pairs.
 * it might visit nodes multiple times.
 * it will not get stuck in a loop.
 */
 void deepwalk(Node* node, void callback (Node*), DupStack* stack){
  switch (node->tag){
    case Tag_App:
      deepwalk(node->s0, callback, stack);
      deepwalk(node->s1, callback, stack);
      break;
    case Tag_Sup:{
      DupStack* head = malloc(sizeof(DupStack));
      DupStack* current = head;
      current->next = stack;
      while (current->next != NULL){

        if (current->next->label == node->label){
          DupStack* matched = current->next;
          current->next = current->next->next;
          deepwalk(matched->is_dup2 ? node->s1 : node->s0, callback, head->next);
          current->next = matched;
          free(head);
          callback(node);
          return;
        }
        current = current->next;
      }
      deepwalk(node->s0, callback, stack);
      deepwalk(node->s1, callback, stack);
      free(head);
      break;
    }
    case Tag_Dup:
    case Tag_Dup2:{
      DupStack newstack = {node->label, node->tag == Tag_Dup2, stack};
      deepwalk(node->s0, callback, &newstack);
      break;
    }
    case Tag_Lam:
      deepwalk(node->s0, callback, stack);
      break;
    case Tag_Null:
    case Tag_Var: break;
  }

  callback(node);
}

