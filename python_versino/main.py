#%%
from enum import Enum, auto

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
  def __init__(self, tag: Tag, s0:"Node" = None, s1:"Node" = None, label:int = None):
    self.tag = tag
    self.s0 = s0
    self.s1 = s1
    self.label = label
  def __str__(self)->str: return format_term(self, {})
  def __repr__(self)->str: return format_term(self, {})



def x(var:int) -> Node: return Node(Tag.intermediate_var, label = var)

def num(n:int) -> Node: return Node(Tag.prim, label = n)

def var(lam:Node) -> Node:
  lam.s1 = Node(Tag.var, lam)
  return lam.s1

def app(func:Node, arg:Node) -> Node: return Node(Tag.app, func, arg)

ilab = 70

def sup(a:Node, b:Node, label:int = None)->Node:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  return Node(Tag.sup, a, b, label)

def dup(s0:Node, label:int = None)->tuple[Node, Node]:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  d = Node(Tag.dup, s0, label = label)
  d2 = Node(Tag.dup2, s0, d, label)
  d.s1 = d2
  d2.s1 = d
  return d, d2

def null(): return Node(Tag.null)


def lam(body:Node) -> Node:
  res = Node(Tag.lam, body)
  parse_lam(res, body, 0)
  return res

def move(src:Node, dst:Node )->Node:
  dst.tag = src.tag
  dst.s0 = src.s0
  dst.s1 = src.s1
  dst.label = src.label
  if src.tag == Tag.var:
    lam = src.s0
    assert lam.tag == Tag.lam
    lam.s1 = dst
  if src.tag == Tag.lam:
    dst.s1.s0 = dst
  if src.tag in [Tag.dup, Tag.dup2]:
    dst.s1.s1 =dst
  return dst

def parse_lam(lam:Node, current:Node, depth:int)->Node:
  match current.tag:
    case Tag.lam:
      parse_lam(lam, current.s0, depth + 1)
    case Tag.intermediate_var if current.label == depth:
      if (lam.s1):
        a,b = dup(Node(Tag.var, lam))
        move(a, lam.s1)
        move(b, current)
        lam.s1 = a.s0
      else:
        lam.s1 = move(Node(Tag.var, lam), current)
    case Tag.app | Tag.sup:
      parse_lam(lam, current.s0, depth)
      parse_lam(lam, current.s1, depth)
    case Tag.dup | Tag.dup2:
      parse_lam(lam, current.s0, depth)

def format_term(term:Node, ctx: dict[Node, int])->str:
  def varname(node:Node):
    if node not in ctx: ctx[node] = chr(len(ctx) + 96)
    return ctx[node]
    
  ctx[None] = ""
  match term.tag:
    case Tag.lam:

      return f"λ{varname(term.s1)}.{format_term(term.s0, ctx)}"
    case Tag.app: return f"({format_term(term.s0, ctx)} {format_term(term.s1, ctx)})"
    case Tag.var: return varname(term)
    case Tag.dup | Tag.dup2:

      d1 = term if (term.tag == Tag.dup) else term.s1
      if d1 in ctx:
        a = varname(d1)
        b = varname(d1.s1)
        return a if term == d1 else b
      a = varname(d1)
      b = varname(d1.s1)
      return f"&{term.label}{{{a},{b}}} = {format_term(term.s0,ctx)}; {a if term==d1 else b}"
    case Tag.sup: return f"&{term.label}{{{format_term(term.s0, ctx)}, {format_term(term.s1, ctx)}}}"
    case Tag.null: return "Nul"
    case Tag.prim: return str(term.label)
  return str(term.tag)


t = lam(lam(app(x(1), x(1))))

t


#%%


def fun(bod:Node)->Node:
  v = Node(Tag.var)
  res = Node(Tag.lam, bod, v)
  v.s0 = res
  return res

def reduce(term:Node):
  if term is None: return
  other = term.s0
  if other is None: return
  print(term, other)
  # reduce(other)
  match (term.tag, other.tag):
    case (Tag.app, Tag.lam):
      arg = term.s1
      if other.s1: move(arg, other.s1)
      move(other.s0, term)
      reduce(term)

    case (Tag.app, Tag.sup):
      dups = dup(term.s1, other.label)
      sp = sup(app(other.s0, dups[0]), app(other.s1, dups[1]), other.label)
      move(sp, term)
      reduce(term)
    case (Tag.dup | Tag.dup2, ot):
      da, db = (term, term.s1) if term.tag == Tag.dup else (term.s1, term)
      match ot:
        case Tag.sup:
          if other.label == da.label:
            move(other.s0, da)
            move(other.s1, db)
            reduce(term)
          else:
            dup1 = dup(other.s0, da.label)
            dup2 = dup(other.s1, da.label)
            move(sup(dup1[0], dup2[0], other.label), da)
            move(sup(dup1[1], dup2[1], other.label), db)
            reduce(term)
        case Tag.lam:
          bods = dup(other.s0, term.label)
          funa, funb = fun(bods[0]), fun(bods[1])
          oldvar = other.s1
          move(sup(funa.s1, funb.s1, term.label), oldvar)
          move(funa, da)
          move(funb, db)
          reduce(term)
        case Tag.prim | Tag.null:
          move(move(other, da), db)
    case (Tag.sup, on):
      reduce(term.s0)
      reduce(term.s1)
    case (Tag.lam, on):
      reduce(term.s0)


t = app(lam(x(0)), null())

print(t)

reduce(t)

print(t)



a,b = dup(sup(null(), num(1),0), 0)
print(a)
reduce(a)
print(a)

a,b = dup(sup(null(), num(1),0), 1)
print(a)
reduce(a)
print(a)

a,b = dup(lam(x(0)))

print(a)
reduce(a)
print(a)

a = app(sup(lam(x(0)), lam(null())), num(0))
print(a)
reduce(a)
print(a)
