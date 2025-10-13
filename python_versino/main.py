#%%
from enum import Enum, auto
from dataclasses import dataclass

class Tag(Enum):
  era = auto()
  null = auto()
  var = auto()
  root = auto()
  prim = auto()

  intermediate_var = auto()

  lam = auto()
  app = auto()
  dup = auto()
  sup = auto()

  def __str__(self)->str: return self.name
  def __repr__(self)->str: return self.name

class Port:
  def __init__(self, node:"Node", aux:bool=False):
    self.node = node
    self.aux = aux
  @property
  def tag(self)->Tag: return self.node.tag
  @property
  def target(self)->"Port": return self.node.target
  @property
  def targetb(self)->"Port": return self.node.targetb
  @property
  def label(self)->int: return self.node.label
  def __repr__(self)->str: return format_term(self, {})
  def copy(self, dst:"Port" = None)->"Port":
    if dst is None: dst = Port(None)
    dst.node, dst.aux = self.node, self.aux
    return dst


class Node:
  def __init__(self, tag: Tag, a:Port = None, b:Port = None, label:int= 0):
    self.tag = tag
    self.target: Port = a
    self.targetb: Port = b
    self.label = label
  def __str__(self)->str: return format_term(self, {})
  def __repr__(self)->str: return format_term(self, {})
  def copy(self, dst:"Node" = None)->"Node":
    if dst is None: dst = Node(None)
    dst.tag, dst.target, dst.targetb, dst.label = self.tag, self.target, self.targetb, self.label
    return dst


def x(depth:int) -> Port: return Port(Node(Tag.intermediate_var, label = depth))
def num(n:int) -> Port: return Port(Node(Tag.prim, label = n))
def app(func:Port, arg:Port) -> Port: return Port(Node(Tag.app, func, arg))

ilab = 70

def sup(a:Port, b:Port, label:int = None)->Port:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  return Port(Node(Tag.sup, a, b, label))

def dup(target:Port, label:int = None)->tuple[Port, Port]:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  node = Node(Tag.dup, target, label = label)
  return Port(node, False), Port(node, True)

def null(): return Port(Node(Tag.null))

def lam(body:tuple[bool, Node]) -> Port:
  node = Port(Node(Tag.lam, body))
  parse_lam(node, node, -1)
  return node

def parse_lam(lam:Port, current:Port, depth:int)->Port:
  node = current.node
  match node.tag:
    case Tag.lam:
      node.target = parse_lam(lam, node.target, depth + 1)
    case Tag.intermediate_var if node.label == depth:
      if (lam.node.targetb):
        dups = dup(Port(lam.node, True))
        dups[0].copy(lam.node.targetb)
        lam.node.targetb = dups[0].target
        return dups[1]
      else:
        lam.node.targetb = Port(lam.node, True)
        return lam.node.targetb
    
    case Tag.app | Tag.sup:
      node.target = parse_lam(lam, node.target, depth)
      node.targetb = parse_lam(lam, node.targetb, depth)
    case Tag.dup:
      node.target = parse_lam(lam, node.target, depth)
  return current

def format_term(term:Port, ctx: dict[Node, int])->str:
  node = term.node
  match term.aux,node.tag:
    case False,Tag.lam:
      ctx[node] = chr(len(ctx) + 97)
      return f"λ{ctx[node]}.{format_term(node.target, ctx)}"
    case True,Tag.lam: return ctx[node]
    case _, Tag.app: return f"({format_term(node.target, ctx)} {format_term(node.targetb, ctx)})"
    case _,Tag.dup: return f"&{node.label}.{format_term(node.target, ctx)}"
    case _,Tag.sup: return f"&{node.label}{{{format_term(node.target, ctx)}, {format_term(node.targetb, ctx)}}}"
    case _,Tag.null: return "Nul"
  return str(node.tag)

#%%



def _lam(term:Node):
  lam = Node(Tag.lam)
  lam.target = term
  lam.targetb = Node(Tag.var)
  return lam

def reduce(term:Node):
  if term is None: return
  other = term.target
  if other is None: return
  reduce(other)
  match (term.tag, other.tag):
    case (Tag.app, Tag.lam):
      arg = term.targetb
      copy(other.target, term)
      copy(arg, other.targetb)
      reduce(term)
    case (Tag.app, Tag.sup):
      dups = dup(term.targetb, other.label)
      sp = sup(app(other.target, dups[0]), app(other.targetb, dups[1]), other.label)
      copy(sp, term)
      reduce(term)
    case (Tag.dup | Tag.dup2, on):
      da, db = term, term.targetb if term.tag == Tag.dup else (term.targetb, term)
      match on:
        case Tag.sup:
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
        case Tag.lam:
          bods = dup(other.target, term.label)
          copy(sup(copy(_lam(bods[0]), da), copy(_lam(bods[1]), db), term.label), other)
        case Tag.prim | Tag.null:
          copy(copy(other, da), db)
    case (Tag.sup, on):
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