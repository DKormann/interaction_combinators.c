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

  intermediate_var = auto()

  lam = auto()
  app = auto()
  dup = auto()
  dup2 = auto()
  sup = auto()

  def __str__(self)->str: return self.name
  def __repr__(self)->str: return self.name

class Node:
  def __init__(self, nodeType: NodeType):
    self.nodeType = nodeType
    self.target = None
    self.targetb = None
    self.label = None
  def __str__(self)->str: return format_term(self, {})
  def __repr__(self)->str: return format_term(self, {})


def lam(body:Node) -> Node:
  res = Node(NodeType.lam)
  res.target = body
  res.label = 0
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

ilab = 70

def sup(a:Node, b:Node, label:int = None)->Node:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  res = Node(NodeType.sup)
  res.target = a
  res.targetb = b
  res.label = label
  return res

def dup(target:Node, label:int = None)->tuple[Node, Node]:
  global ilab
  if label is None: label = (ilab := ilab + 1)
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


def parse_lam(lam:Node, current:Node, depth:int)->Node:
  match current.nodeType:
    case NodeType.lam:
      parse_lam(lam, current.target, depth + 1)
    case NodeType.intermediate_var if current.label == depth:
      if (lam.targetb):
        raise NotImplemented()
        # prev_var = lam.targetb
        # lam.targetb = copy(prev_var)
        # a,b = dup(lam.targetb)
        # copy(a, prev_var)
        # copy(b,current)
      else:
        return lam
        
    case NodeType.dup | NodeType.dup2 | NodeType.app | NodeType.sup:
      parse_lam(lam, current.target, current, depth)
      parse_lam(lam, current.targetb, current, depth)

def format_term(term:Node, ctx: dict[Node, int])->str:
  ctx[None] = "_"
  match term.nodeType:
    case NodeType.lam:
      if term.targetb: ctx[term.targetb] = chr(len(ctx) + 96)
      return f"λ{ctx[term.targetb]}.{format_term(term.target, ctx)}"
    case NodeType.app:
      return f"({format_term(term.target, ctx)} {format_term(term.targetb, ctx)})"
    case NodeType.var:
      if term not in ctx: ctx[term] = chr(len(ctx) + 96)
      return ctx[term]
    case NodeType.dup | NodeType.dup2:
      return f"&{term.label}.{format_term(term.target, ctx)}"
    case NodeType.sup:
      return f"&{term.label}{{{format_term(term.target, ctx)}, {format_term(term.targetb, ctx)}}}"
    case NodeType.null:
      return "Nul"
  return str(term.nodeType)


t = lam(lam(app(x(1), x(1))))
t

#%%


def copy(src:Node, dst:Node = None)->Node:
  if dst is None: dst = Node(None)
  dst.nodeType = src.nodeType
  if src.target: dst.target = src.target
  if src.label is not None: dst.label = src.label
  if src.targetb: dst.targetb = src.targetb
  return dst

def _lam(term:Node):
  lam = Node(NodeType.lam)
  lam.target = term
  lam.targetb = Node(NodeType.var)
  return lam

def reduce(term:Node):
  if term is None: return
  other = term.target
  if other is None: return
  reduce(other)
  match (term.nodeType, other.nodeType):
    case (NodeType.app, NodeType.lam):
      arg = term.targetb
      copy(other.target, term)
      copy(arg, other.targetb)
      reduce(term)
    case (NodeType.app, NodeType.sup):
      dups = dup(term.targetb, other.label)
      sp = sup(app(other.target, dups[0]), app(other.targetb, dups[1]), other.label)
      copy(sp, term)
      reduce(term)
    case (NodeType.dup | NodeType.dup2, on):
      da, db = term, term.targetb if term.nodeType == NodeType.dup else (term.targetb, term)
      match on:
        case NodeType.sup:
          if other.label == da.label:
            copy(other.target, da)
            copy(other.targetb, db)
            reduce(term)
          else:
            dup1 = dup(other.target, da.label)
            dup2 = dup(other.targetb, da.label)
            copy(sup(dup1[0], dup2[0], other.label), da)
            copy(sup(dup1[1], dup2[1], other.label), db)
            reduce(term)
        case NodeType.lam:
          bods = dup(other.target, term.label)
          copy(sup(copy(_lam(bods[0]), da), copy(_lam(bods[1]), db), term.label), other)
        case NodeType.prim | NodeType.null:
          copy(copy(other, da), db)
    case (NodeType.sup, on):
      reduce(term.targetb)


t = app(lam(x(0)), null())

print(t)

reduce(t)

print(t)

#%%




def l0(): return lam(lam(x(0)))
def l1(): return lam(lam(x(1)))

a0 = app(l1(), null())
print(a0)
reduce(a0)
a0

#%%

a,b = dup(sup(null(), null(),0), 0)
print(a)
reduce(a)
print(a)

a,b = dup(sup(null(), num(1), 0), 1)
print(a)
reduce(a)
print(a)



# %%
a