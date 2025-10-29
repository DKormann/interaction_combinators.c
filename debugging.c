
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
