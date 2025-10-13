#%%

from code import interact
from enum import Enum, auto
from dataclasses import dataclass
from tarfile import tar_filter
from threading import currentThread


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
  dup2 = auto()
  sup = auto()

  def __str__(self)->str: return self.name
  def __repr__(self)->str: return self.name

class Node:
  def __init__(self, tag: Tag):
    self.tag = tag
    self.s0 = None
    self.s1 = None
    self.label = None
  def __str__(self)->str: return format_term(self, {})
  def __repr__(self)->str: return format_term(self, {})


def lam(body:Node) -> Node:
  res = Node(Tag.lam)
  res.s0 = body
  res.label = 0
  parse_lam(res, body, 0)
  return res

def x(var:int) -> Node:
  res = Node(Tag.intermediate_var)
  res.label = var
  return res

def num(n:int) -> Node:
  res = Node(Tag.prim)
  res.label = n
  return res

def app(func:Node, arg:Node) -> Node:
  res = Node(Tag.app)
  res.s0 = func
  res.s1 = arg
  return res

ilab = 70

def sup(a:Node, b:Node, label:int = None)->Node:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  res = Node(Tag.sup)
  res.s0 = a
  res.s1 = b
  res.label = label
  return res

def dup(s0:Node, label:int = None)->tuple[Node, Node]:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  d = Node(Tag.dup)
  d2 = Node(Tag.dup2)
  d.s0 = d2.s0 = s0
  d.s1 = d2
  d2.s1 = d
  d.label = label
  d2.label = label
  return d, d2

def null():
  return Node(Tag.null)


def parse_lam(lam:Node, current:Node, depth:int)->Node:
  match current.tag:
    case Tag.lam:
      parse_lam(lam, current.s0, depth + 1)
    case Tag.intermediate_var if current.label == depth:
      if (lam.s1):
        raise NotImplemented()
        # prev_var = lam.s1
        # lam.s1 = copy(prev_var)
        # a,b = dup(lam.s1)
        # copy(a, prev_var)
        # copy(b,current)
      else:
        return lam
        
    case Tag.dup | Tag.dup2 | Tag.app | Tag.sup:
      parse_lam(lam, current.s0, current, depth)
      parse_lam(lam, current.s1, current, depth)

def format_term(term:Node, ctx: dict[Node, int])->str:
  ctx[None] = "_"
  match term.tag:
    case Tag.lam:
      if term.s1: ctx[term.s1] = chr(len(ctx) + 96)
      return f"λ{ctx[term.s1]}.{format_term(term.s0, ctx)}"
    case Tag.app:
      return f"({format_term(term.s0, ctx)} {format_term(term.s1, ctx)})"
    case Tag.var:
      if term not in ctx: ctx[term] = chr(len(ctx) + 96)
      return ctx[term]
    case Tag.dup | Tag.dup2:
      return f"&{term.label}.{format_term(term.s0, ctx)}"
    case Tag.sup:
      return f"&{term.label}{{{format_term(term.s0, ctx)}, {format_term(term.s1, ctx)}}}"
    case Tag.null:
      return "Nul"
  return str(term.tag)


t = lam(lam(app(x(1), x(1))))
t

#%%


def copy(src:Node, dst:Node = None)->Node:
  if dst is None: dst = Node(None)
  dst.tag = src.tag
  if src.s0: dst.s0 = src.s0
  if src.label is not None: dst.label = src.label
  if src.s1: dst.s1 = src.s1
  return dst

def _lam(term:Node):
  lam = Node(Tag.lam)
  lam.s0 = term
  lam.s1 = Node(Tag.var)
  return lam

def reduce(term:Node):
  if term is None: return
  other = term.s0
  if other is None: return
  reduce(other)
  match (term.tag, other.tag):
    case (Tag.app, Tag.lam):
      arg = term.s1
      copy(other.s0, term)
      copy(arg, other.s1)
      reduce(term)
    case (Tag.app, Tag.sup):
      dups = dup(term.s1, other.label)
      sp = sup(app(other.s0, dups[0]), app(other.s1, dups[1]), other.label)
      copy(sp, term)
      reduce(term)
    case (Tag.dup | Tag.dup2, on):
      da, db = term, term.s1 if term.tag == Tag.dup else (term.s1, term)
      match on:
        case Tag.sup:
          if other.label == da.label:
            copy(other.s0, da)
            copy(other.s1, db)
            reduce(term)
          else:
            dup1 = dup(other.s0, da.label)
            dup2 = dup(other.s1, da.label)
            copy(sup(dup1[0], dup2[0], other.label), da)
            copy(sup(dup1[1], dup2[1], other.label), db)
            reduce(term)
        case Tag.lam:
          bods = dup(other.s0, term.label)
          copy(sup(copy(_lam(bods[0]), da), copy(_lam(bods[1]), db), term.label), other)
        case Tag.prim | Tag.null:
          copy(copy(other, da), db)
    case (Tag.sup, on):
      reduce(term.s1)


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