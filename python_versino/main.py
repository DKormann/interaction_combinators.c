#%%

from enum import Enum, auto
from string import Formatter

class Tag(Enum):
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

hide_dups = False
print_tree = True

class Node:
  def __init__(self, tag: Tag, s0:"Node" = None, s1:"Node" = None, label:int = None):
    self.tag = tag
    self.s0 = s0
    self.s1 = s1
    self.label = label
  def __str__(self)->str: return (tree if print_tree else format_term)(self, {})
  def __repr__(self)->str: return str(self)

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
    case Tag.lam: return f"λ{varname(term.s1)}.{format_term(term.s0, ctx)}"
    case Tag.app: return f"({format_term(term.s0, ctx)} {format_term(term.s1, ctx)})"
    case Tag.var: return varname(term)
    case Tag.dup | Tag.dup2:
      if hide_dups: return format_term(term.s0, ctx)
      d1 = term if (term.tag == Tag.dup) else term.s1
      if d1 in ctx: return varname(d1 if term == d1 else d1.s1)
      a = varname(d1)
      b = varname(d1.s1)
      return f"&{{{a},{b}}} = {format_term(term.s0,ctx)}; {a if term==d1 else b}"
    case Tag.sup: return f"&{{{format_term(term.s0, ctx)}, {format_term(term.s1, ctx)}}}"
    case Tag.null: return "Nul"
    case Tag.prim: return str(term.label)
  return str(term.tag)

def tree(term:Node, ctx:dict[Node, int])->str:
  def varname(node:Node | None):
    if node is None: return ""
    return ctx.setdefault(node, chr(len(ctx) + 97))
  def idn(lns:list[str])->list[str]:
    if sum(len(ln) for ln in lns) <= 20: return ["  " + " ".join(map(str.strip, lns))]
    return ["  " + ln for ln in lns]
  def prep(head:str, lns:list[str])->list[str]: return [head + " " + lns[0].strip()] + lns[1:]
  def _tree(term:Node)->list[str]:
    match term.tag:
      case Tag.lam: return prep(f"λ{varname(term.s1)}", _tree(term.s0))
      case Tag.app | Tag.sup: return [term.tag.name + (f"{term.label}" if term.label else "")] + idn(_tree(term.s0)) + idn(_tree(term.s1))
      case Tag.dup | Tag.dup2:
        if hide_dups: return _tree(term.s0)
        d1 = term if (term.tag == Tag.dup) else term.s1
        if d1 in ctx: return [varname(term)]
        return [f"{term.label}{{{varname(d1)}, {varname(d1.s1)}}} ="] + idn(_tree(term.s0) + [f"in {varname(term)}"])
      case Tag.prim: return [str(term.label)]
    return [varname(term)]
  return "\n".join(_tree(term))


# %%



DEBUG = False

def expect_output(term:Node, output:str):
  if isinstance(output, Node): output = format_term(output, {})
  global hide_dups, print_tree, DEBUG
  prev_hide_dups, prev_print_tree, prev_DEBUG = hide_dups, print_tree, DEBUG
  hide_dups, print_tree, DEBUG= False, False, False
  init = format_term(term, {})
  reduce(term)
  res = format_term(term, {})
  assert res == output, f"reduced: {init} -> {res} != {output}"
  hide_dups, print_tree, DEBUG = prev_hide_dups, prev_print_tree, prev_DEBUG

def church_true(): return lam(lam(x(1)))
def church_false(): return lam(lam(x(0)))

def church_nat(n:int):
  def go(n:int): return x(0) if n == 0 else app(x(1), go(n-1))
  return lam(lam(go(n)))

def test_reduce():
  expect_output(lam(x(0)), "λa.a")
  expect_output(null(), "Nul")
  expect_output(church_true(), "λa.λ.a")
  expect_output(church_nat(0), "λ.λa.a")
  expect_output(church_nat(1), "λa.λb.(a b)")
  expect_output(church_nat(2), "λa.λb.(&{c,d} = a; c (d b))")

  expect_output(dup(sup(null(), num(1), 0), 0)[0], "Nul")
  expect_output(dup(sup(null(), num(1), 0), 1)[0], "&{Nul, 1}")
  expect_output(dup(lam(x(0)))[0], "λa.a")
  expect_output(app(sup(lam(x(0)), lam(null())), num(2)), "&{2, Nul}")
  expect_output(app(sup(lam(x(0)), lam(x(0)), 0), sup(num(1), num(2), 0)), "&{1, 2}")

  expect_output(dup(lam(x(0)))[0], "λa.a")
  expect_output(dup(lam(sup(num(1), num(2), 0)),0)[0], "λa.1")
  expect_output(dup(lam(sup(num(1), num(2), 0)),1)[0], "λa.&{1, 2}")

  expect_output(
    app(church_nat(2), lam(x(0))),
    "λa.a"
  )

  expect_output(
    app(lam(x(0)), lam(x(0))),
    "λa.a"
  )


  expect_output(
    app(lam(x(0)), church_nat(2)),
    "λa.λb.(&{c,d} = a; c (d b))"
  )

  expect_output(
    app(lam(sup(x(0), app(x(0), num(1)))), lam(x(0))),
    sup(lam(x(0)), num(1))
  )

  expect_output(
    app(church_nat(1), church_nat(1)),
    "λa.λb.(a b)"
  )

  expect_output(
    app(church_nat(1), church_nat(2)),
    "λa.λb.(&{c,d} = a; c (d b))"
  )

  expect_output(
    app(church_nat(2), church_nat(1)),
    "λa.λb.(a &{c,d} = &{b, c}; d)"
  )
  

#%%
def fun(bod:Node)->Node:
  v = Node(Tag.var)
  res = Node(Tag.lam, bod, v)
  v.s0 = res
  return res


def debug(*args):
  global DEBUG
  if DEBUG: print(*args)


def step(term:Node)->bool:
  if term.tag in [Tag.var, Tag.prim, Tag.null]: return
  other = term.s0
  if other is None: return
  debug("STEP:", term)
  match (term.tag, other.tag):
    case (Tag.app, Tag.lam):
      if other.s1: move(term.s1, other.s1)
      move(other.s0, term)
      return True
    case (Tag.app, Tag.sup):
      da, db = dup(term.s1, other.label)
      move(sup(app(other.s0, da), app(other.s1, db), other.label), term)
      return True
    case (Tag.app, Tag.dup | Tag.dup2):
      return step(other)
    case (Tag.app, Tag.prim):
      match term.s1.tag:
        case Tag.prim:
          res = other.label(term.s1.label)
          move(num(res), term)
          return True
        case Tag.sup:
          pass
      return False

    case (Tag.app, _):
      return step(other)
    case (Tag.dup | Tag.dup2, other_tag):
      da, db = (term, term.s1) if term.tag == Tag.dup else (term.s1, term)
      match other_tag:
        case Tag.sup:
          debug("STEP: dup | dup2 -> sup")
          if other.label == da.label:
            move(other.s0, da)
            move(other.s1, db)
          else:
            dup1 = dup(other.s0, da.label)
            dup2 = dup(other.s1, da.label)
            move(sup(dup1[0], dup2[0], other.label), da)
            move(sup(dup1[1], dup2[1], other.label), db)
          return True
        case Tag.lam:
          debug("STEP: dup | dup2 -> lam")
          ba, bb = dup(other.s0, term.label)
          funa, funb = fun(ba), fun(bb)
          if other.s1: move(sup(funa.s1, funb.s1, term.label), other.s1)
          move(funa, da)
          move(funb, db)
          return True
        case Tag.app:
          return step(other)
        case Tag.prim | Tag.null:
          move(move(other, da), db)
          return True
        case Tag.var: return False
        case _:
          return step(other)
    case (Tag.sup, on):
      return step(term.s0) or step(term.s1)
    case (Tag.lam, on):
      return step(term.s0)
  
  debug("NO interaction: ", term.tag, other.tag)
  return False


def reduce(term):
  while step(term):
    pass



c4 = app(church_nat(2), church_nat(2))

reduce(c4)
print(c4)


#%%

import sys
sys.setrecursionlimit(30)

DEBUG = False

suc = num(lambda x: x + 1)

a = app(app(c4, suc), num(1))

# reduce(a)

step(a)
print(a)

step(a)
print(a)


hide_dups = True
step(a)
print(a)




