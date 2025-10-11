
#%%

from code import interact
from enum import Enum, auto
from dataclasses import dataclass
from tarfile import tar_filter
from threading import currentThread



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
  era = auto()
  null = auto()
  var = auto()
  root = auto()
  prim = auto()

  lam = auto()
  app = auto()
  dup = auto()
  dup2 = auto()
  sup = auto()

  intermediate_var = auto()
  def __str__(self)->str:
    return self.name
  def __repr__(self)->str:
    return self.name

class Node:
  def __init__(self, nodeType: NodeType):
    self.nodeType = nodeType
    self.target : Node | None = None
    self.label : int | None = None
    self.targetb : Node | None = None
  def __str__(self)->str:
    # if self.nodeType in [NodeType.lam, NodeType.app, NodeType.dup, NodeType.sup]:
    #   return format_term(self, {})
    # return str(self.nodeType)
    return format_term(self, {})
  def __repr__(self)->str: return format_term(self, {})

def parse_lam(lam:Node, current:Node, depth:int)->Node:
  match current.nodeType:
    case NodeType.lam:
      parse_lam(lam, current.target, depth + 1)
    case NodeType.intermediate_var if current.label == depth:
      if (lam.targetb):
        var = Node(NodeType.var)
        a,b = dup(var)
        copy_node(a, lam.targetb)
        copy_node(b, current)
        lam.targetb = var
        var.target = lam
      else:
        lam.targetb = current
        current.nodeType = NodeType.var
        current.target = lam
    case NodeType.app:
      parse_lam(lam, current.target, depth)
      parse_lam(lam, current.targetb, depth)
    case NodeType.dup | NodeType.dup2:
      parse_lam(lam, current.target, depth)
      parse_lam(lam, current.targetb, depth)
    case NodeType.sup:
      parse_lam(lam, current.target, depth)
      parse_lam(lam, current.targetb, depth)

def lam(body:Node) -> Node:
  res = Node(NodeType.lam)
  res.target = body
  parse_lam(res, body, 0)
  return res

def x(var:int) -> Node:
  res = Node(NodeType.intermediate_var)
  res.label = var
  return res

def num(n:int) -> Node:
  res = Node(NodeType.prim)
  res.label = n
  return res

def app(func:Node, arg:Node) -> Node:
  res = Node(NodeType.app)
  res.target = func
  res.targetb = arg
  return res

def sup(a:Node, b:Node, label:int = 0)->Node:
  res = Node(NodeType.sup)
  res.target = a
  res.targetb = b
  res.label = label
  return res

def dup(target:Node, label:int = 0)->tuple[Node, Node]:
  d = Node(NodeType.dup)
  d2 = Node(NodeType.dup2)
  d.target = d2.target = target
  d.targetb = d2
  d2.targetb = d
  d.label = label
  d2.label = label
  return d, d2

def null():
  return Node(NodeType.null)

def format_term(term:Node, ctx: dict[Node, int])->str:

  match term.nodeType:
    case NodeType.lam:
      if not term in ctx: ctx[term] = len(ctx)
      return f"λ{chr(ctx[term] + 97)}.{format_term(term.target, ctx)}"
    case NodeType.app:
      return f"({format_term(term.target, ctx)} {format_term(term.targetb, ctx)})"
    case NodeType.var:
      if term.target.nodeType == NodeType.lam:
        return f"{chr(ctx[term.target] + 97)}"
      return format_term(term.target, ctx)
    case NodeType.dup | NodeType.dup2:
      return f"&{term.label}.{format_term(term.target, ctx)}"
    case NodeType.sup:
      return f"&{term.label}{{{format_term(term.target, ctx)}, {format_term(term.targetb, ctx)}}}"
    case NodeType.null:
      return "Nul"
  return str(term.nodeType)


x1,x2 = dup(null())
sup(lam(x(0)), x1)

#%%

lam(app(x(0), x(0))).target.target.target.nodeType
#%%


def copy_node(src:Node, dst:Node = None)->Node:
  if dst is None: dst = Node(None)
  dst.nodeType = src.nodeType
  dst.target = src.target
  if src.label: dst.label = src.label
  if src.targetb: dst.targetb = src.targetb
  return dst

def _lam(term:Node):
  lam = Node(NodeType.lam)
  lam.target = term
  return lam

def reduce(term:Node):
  other = term.target
  match term.nodeType:
    case NodeType.app:
      func = term.target
      match func.nodeType:
        case NodeType.lam:
          arg = term.targetb
          ret = func.target
          if func: copy_node(arg, func)
          copy_node(ret, term)
        case NodeType.sup:
          d = dup()
    case NodeType.dup | NodeType.dup2:
      d1,d2 = (term, term.targetb) if term.nodeType == NodeType.dup else (term.targetb, term)
      match d1.target.nodeType:
        case NodeType.sup:
          if other.label == d2.label:
            copy_node(other.target, d1)
            copy_node(other.targetb, d2)
          else:
            da = dup(other.target, d1.label)
            db = dup(other.targetb, d1.label)
            copy_node(sup(da[0], db[0], other.label), d1)
            copy_node(sup(da[1], db[1], other.label), d2)
        case NodeType.lam:
          bods = dup(other.target)
          l1 = _lam(bods[0])
          l2 = _lam(bods[1])
          copy_node(sup(l1, l2, d1.label), other)
        case NodeType.prim:
          copy_node(copy_node(d1.target), d2)
          copy_node(d1.target, d1)
    case NodeType.lam:
      reduce(term.target)
    case NodeType.sup:
      reduce(term.target)
      reduce(term.targetb)
    





def l0(): return lam(lam(x(0)))
def l1(): return lam(lam(x(1)))

a0 = app(l1(), null())
print(a0)
reduce(a0)
a0
#%%
a,b = dup(sup(null(), null()))
print(a)
reduce(a)
print(a)

a,b = dup(sup(null(), num(1), 1))
print(a)
reduce(a)
print(a)



# %%
a