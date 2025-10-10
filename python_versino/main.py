
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
  lam = auto()
  app = auto()
  dup = auto()
  dup2 = auto()
  sup = auto()
  sup2 = auto()
  root = auto()
  var = auto()
  intermediate_var = auto()
  def __str__(self)->str:
    return self.name
  def __repr__(self)->str:
    return self.name

ERA = NodeType.era
NULL = NodeType.null
LAM = NodeType.lam
APP = NodeType.app
DUP = NodeType.dup
DUP2 = NodeType.dup2
SUP = NodeType.sup
SUP2 = NodeType.sup
ROOT = NodeType.root

class Node:
  def __init__(self, nodeType: NodeType):
    self.nodeType = nodeType
    self.target : Node | None = None
    self._label : int | None = None
    self._targetb : Node | None = None

  @property
  def label(self)->int:
    if self.nodeType in [NodeType.dup, NodeType.sup, NodeType.intermediate_var]:
      return self._label
    return None
  
  @label.setter
  def label(self, value:int):
    if self.nodeType in [NodeType.dup, NodeType.sup, NodeType.intermediate_var]:
      self._label = value
    else: raise ValueError("Label can only be set for dup,  and sup nodes")
  
  @property
  def targetb(self)->"Node":
    # if (self.nodeType == NodeType.sup or self.nodeType == NodeType.app):
    return self._targetb
    # return None
  
  @targetb.setter
  def targetb(self, value:"Node"):
    # if (self.nodeType == NodeType.sup or self.nodeType == NodeType.app):
    self._targetb = value
    # else:
      # raise ValueError("Targetb can only be set for sup nodes")
  
  def __str__(self)->str:
    if self.nodeType in [NodeType.lam, NodeType.app, NodeType.dup, NodeType.sup]:
      return format_term(self, {})
    return str(self.nodeType)
  def __repr__(self)->str:
    return format_term(self, {})

def parse_lam(lam:Node, current:Node, depth:int)->Node:
  match current.nodeType:
    case NodeType.lam:
      parse_lam(lam, current.targetb, depth + 1)
    case NodeType.intermediate_var:
      if current.label == depth:
        current.nodeType = NodeType.var
        lam.target = current if lam.target is None else dup(lam.target, current, current.label)
    case NodeType.app:
      parse_lam(lam, current.target, depth)
      parse_lam(lam, current.targetb, depth)
    # case NodeType.var:
    #   if current.target == depth:
    #     current.lam = lam
    case NodeType.dup:
      parse_lam(lam, current.target, depth)
      parse_lam(lam, current.targetb, depth)
    case NodeType.sup | NodeType.sup2:
      parse_lam(lam, current.target, depth)

def lam(body:Node) -> Node:
  res = Node(NodeType.lam)
  res.targetb = body
  parse_lam(res, body, 0)
  return res

def x(var:int) -> Node:
  res = Node(NodeType.intermediate_var)
  res.label = var
  return res

def app(func:Node, arg:Node) -> Node:
  res = Node(NodeType.app)
  res.target = func
  res.targetb = arg
  return res

def dup(a:Node, b:Node, label:int)->Node:
  res = Node(NodeType.dup)
  res.target = a
  res.targetb = b
  res.label = label
  return res

# def sup(target:Node, label:int)->tuple[Node, Node]:
#   s = Node(NodeType.sup)
#   s2 = Node(NodeType.sup2)
#   s.target = s2.target = target
#   s.targetb = s2
#   s2.targetb = s
#   s.label = label
#   return s, s2

def sup(a:Node, b:Node, label)

def null():
  return Node(NodeType.null)

def format_term(term:Node, ctx: dict[Node, int])->str:

  match term.nodeType:
    case NodeType.lam:
      if not term in ctx: ctx[term] = len(ctx)
      def recname(var:Node):
        match var.nodeType:
          case NodeType.var:
            ctx[var] = ctx[term]
          case NodeType.dup:
            recname(var.target)
            recname(var.targetb)
      if term.target: recname(term.target)
      # if not term.target in ctx: ctx[term.target] = len(ctx)

      return f"λ{chr(ctx[term] + 97)}.{format_term(term.targetb, ctx)}"
    case NodeType.app:
      return f"({format_term(term.target, ctx)} {format_term(term.targetb, ctx)})"
    case NodeType.var:
      return f"{chr(ctx[term] + 97)}"
    case NodeType.dup:
      return format_term(term.target, ctx)
    case NodeType.sup | NodeType.sup2:
      other = term.targetb.target
      label = term.label or other.label
      return f"&{label}{{{format_term(term.target, ctx)},{format_term(term.targetb.target, ctx)}}}"
    case NodeType.null:
      return "Nul"
  return str(term.nodeType)


sup(lam(x(0)), null())

#%%


def copy_node(src:Node, dst:Node):
  dst.nodeType = src.nodeType
  dst.target = src.target
  if src.label: dst.label = src.label
  if src.targetb: dst.targetb = src.targetb

def reduce(term:Node):
  match term.nodeType:
    case NodeType.lam: reduce(term.target)
    case NodeType.app:
      func = term.target
      match func.nodeType:
        case NodeType.lam:
          arg = term.targetb
          ret = func.targetb
          if func.target: copy_node(arg, func.target)
          copy_node(ret, term)
        case NodeType.dup:
          raise NotImplementedError("dup not implemented")
    # case NodeType.sup | NodeType.sup2:
    #   reduce(term.target)
    # case NodeType.dup:
    #   reduce(term.target)


def l0(): return lam(lam(x(0)))
def l1(): return lam(lam(x(1)))

a0 = app(l1(), null())
print(a0)
reduce(a0)
a0

# %%


lam(app(x(0), x(0)))

#%%

sup(lam(x(0)), null())

