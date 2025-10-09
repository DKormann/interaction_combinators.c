


enum PortType {
  Main,
  Aux1,
  Aux2,
}

enum NodeType{
  Era,
  Null,
  Lam,
  App,
  Dup,
  Sup,
  Root,
  IntermediateVar,
}


struct Port {
  location: usize,
  port_type: PortType,
}

struct Node {
  node_type: NodeType,
  main: Port,
  aux1: Port,
  aux2: Port,
  label: char,
}






fn main() {
  println!("Hello, world!");
}
