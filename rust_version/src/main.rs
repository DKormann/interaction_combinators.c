#[derive(Debug)]
enum Node {
  Era,
  Null,
  Lam { body: Box<Node> },
  Var (Var),
  App { func: Box<Node>, arg: Box<Node> },
  Dup1 (Dup1),
  Dup2 (Dup2),
  Sup {a: Box<Node>, b: Box<Node>, label: u8},
}

#[derive(Debug)]
struct Var{
  lam: Box<Node>,
}

#[derive(Debug)]
struct Dup2{
  target: Dup1
}

#[derive(Debug)]
struct Dup1{
  target: Box<Node>
}


fn main() {
  let mut node = Node::Lam { body: Box::new(Node::Null) };

  if let Node::Lam { body } = &mut node {
    // *body = Box::new(Node::Var(Var{lam: Box::new(Node::Null)}));
    *body = Box::new(Node::Var(Var{lam: Box::new(node)}));


  }

  println!("{:?}", node);
}
