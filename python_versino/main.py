
#%%

from enum import Enum
from dataclasses import dataclass



class PortType(Enum):
  main = 0
  aux1 = 1
  aux2 = 2
  def __str__(self) -> str:
    return f"{self.name}"

MAIN = PortType.main
AUX1 = PortType.aux1
AUX2 = PortType.aux2

class NodeType(Enum):
  era = 0
  null = 1
  lam = 2
  app = 3
  dup = 4
  sup = 5
  root = 6
  intermediate_var = 7
  def __str__(self)->str:
    return self.name

ERA = NodeType.era
NULL = NodeType.null
LAM = NodeType.lam
APP = NodeType.app
DUP = NodeType.dup
SUP = NodeType.sup
ROOT = NodeType.root

  
@dataclass
class Port:
  node: "Node"
  port: PortType

  def polarity(self)->bool:
    match self.node.nodeType:
      case NodeType.era | NodeType.root:
        return True
      case NodeType.null | NodeType.intermediate_var:
        return False
      case NodeType.lam:
        return True if self.port == PortType.aux2 else False
      case NodeType.app:
        return False if self.port == PortType.aux2 else True
      case NodeType.dup:
        return True if self.port == PortType.main else False
      case NodeType.sup:
        return False if self.port == PortType.main else True
  
  # def other(self)->"Port":
  #   return self.node.get_port(self.port_type)
  
  @property
  def Main(self)->"Node":
    return self.node.Main
  @property
  def Aux1(self)->"Node":
    return self.node.Aux1
  @property
  def Aux2(self)->"Node":
    return self.node.Aux2
  @property
  def main(self)->"Node":
    return self.node.main
  @property
  def aux1(self)->"Node":
    return self.node.aux1
  @property
  def aux2(self)->"Node":
    return self.node.aux2
  
  def __repr__(self):
    return f"<{self.node.nodeType} {self.port}>"

  def replace(self, other:"Port"):
    self.other.other = other
    other.other = self.other
    self.other = None
  
def polarity(node: NodeType, port: PortType)->bool:
  match node:
    case NodeType.era | NodeType.root:
      return True
    case NodeType.null | NodeType.intermediate_var:
      return False
    case NodeType.lam:
      return True if port == PortType.aux2 else False
    case NodeType.app:
      return not polarity(NodeType.lam, port)
    case NodeType.dup:
      return True if port == PortType.main else False
    case NodeType.sup:
      return not polarity(NodeType.dup, port)

class Node:
  def __init__(self, nodeType:NodeType, label:int = 0):
    self.nodeType = nodeType
    self.main: Port = Port(None, None)
    self.aux1: Port = Port(None, None)
    self.aux2: Port = Port(None, None)
    self.label: int = label
    self.arity = 1 if nodeType in [NodeType.era, NodeType.null, NodeType.root, NodeType.intermediate_var] else 3

  def get_port(self, portType:PortType)->Port:
    match portType:
      case PortType.main: return self.main
      case PortType.aux1: return self.aux1
      case PortType.aux2: return self.aux2
    raise ValueError(f"Invalid port type: {portType}")
  
  def ports(self)->list[Port]: return [self.main, self.aux1, self.aux2][:self.arity]
  
  def connect(self, other:"Node", self_port: PortType, other_port: PortType):
    if (polarity(self.nodeType, self_port) != polarity(other.nodeType, other_port)):
      raise ValueError(f"Polarities do not match: {self} {self_port} vs {other} {other_port}")
    if (self is other and (self_port == MAIN or other_port == MAIN)):
      raise ValueError(f"Cannot connect main ports of the same node: {self}")

    self.get_port(self_port).node = other
    self.get_port(self_port).port = other_port
    other.get_port(other_port).node = self
    other.get_port(other_port).port = self_port
  
  @property
  def Main(self)->"Node":
    return self.main.other.node
  @property
  def Aux1(self)->"Node":
    return self.aux1.other.node
  @property
  def Aux2(self)->"Node":
    return self.aux2.other.node

  def __repr__(self):
    def view_port(port: Port)->str:
      if port.other:
        return f"{port.other.node.nodeType}"
      return "None"
    return f'''<{self.nodeType}: { self.label if self.nodeType in [NodeType.dup, NodeType.sup, NodeType.intermediate_var] else ""
    }{" ".join([view_port(self.main), view_port(self.aux1), view_port(self.aux2)]) if self.arity == 3 else ""}>'''




def root(term:Port)->Port:
  node = Node(NodeType.root)
  node.connect(term.node, MAIN, term.port)
  return node.main


def x(n:int)->Node:
  return Node(NodeType.intermediate_var, label = n)


def parse_intermidiates(current: Port, lam:Node, depth: int, ctx: dict[Node, bool]):
  if ctx.get(current.node): return
  ctx[current.node] = True
  match current.node.nodeType, current.port:
    case NodeType.lam, PortType.main:
      parse_intermidiates(current.node.aux2.other, lam, depth + 1, ctx)
    case NodeType.intermediate_var, _:
      if (current.node.label == depth):
        return current.replace(lam.aux1)
    case NodeType.app, PortType.aux2:
      parse_intermidiates(current.node.main, current.node.aux1, depth, ctx)


def era(term:Node, port:PortType)->Port:
  node = Node(NodeType.era)
  node.connect(term.node, MAIN, term.port)
  return node.main


def lam(body: Port)->Port:
  lam = Node(NodeType.lam)
  lam.aux2.connect(body.node, MAIN, body.port)
  root(lam.main)
  era(lam.aux1)
  parse_intermidiates(lam.aux2.other, lam, 0, {})
  return lam.main

def app(f: Port, x: Port)->Port:
  app = Node(NodeType.app)
  app.main.connect(f)
  app.aux1.connect(x)
  root(app.aux2)
  return app.aux2


lam(x(0))


#%%




l0 = lam(lam(x(0)))
l1 = lam(lam(x(1)))


l1

#%%

def parse_term(term: Port, lam_map: dict[Node, int] = None)->Port:
  if lam_map is None: lam_map = {}
  node = term.node
  match term.node.nodeType:
    case NodeType.lam:
      if (term.port == PortType.main):
        lab = len(lam_map)
        lam_map[term.node] = lab
        return f"λ{chr(lab + 97)}.{parse_term(term.node.aux2.other, lam_map)}"
      if (term.port == PortType.aux1):
        return f"{chr(lam_map[term.node] + 97)}"

    case NodeType.app:
      if (term.port == PortType.aux2):
        return f"({parse_term(node.main.other, lam_map)} {parse_term(node.aux1.other, lam_map)})"
  return f"{term}"

print(parse_term(l0, {}))

#%%


c0 = lam(lam(x(0)))

print(parse_term(c0))
c1 = lam(lam(app(x(1), x(0))))
parse_term(c1)
# %%
