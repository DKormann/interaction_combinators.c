
from enum import Enum, auto
from string import Formatter

class Tag(Enum):
  Null = auto()
  Var = auto()
  Root = auto()
  Prim = auto()

  intermediate_var = auto()

  Lam = auto()
  App = auto()
  Dup = auto()
  Dup2 = auto()
  Sup = auto()

  def __str__(self)->str: return self.name
  def __repr__(self)->str: return self.name


import os
class Env:
  def __init__(self, name:str, value:int):
    self.name = name

    if name in os.environ:
      self.value = int(os.environ[name])
    else:
      self.value = value
  
  def set(self, value:int): self.value = value
  def get(self)->int: return self.value
  def __str__(self)->str: return f"{self.value}"
  def __bool__(self)->bool: return bool(self.value)
  def __int__(self)->int: return int(self.value)
  def __lt__(self, other)->bool: return self.value < int(other)
  def __eq__(self, other)->bool: return self.value == int(other)


hide_dups = Env("hide_dups", False)
print_tree = Env("print_tree", True)
DEBUG = Env("DEBUG", False)


class Node:
  def __init__(self, tag: Tag, s0:"Node" = None, s1:"Node" = None, label:int = None):
    self.tag = tag
    self.s0 = s0
    self.s1 = s1
    self.label = label
  def __str__(self)->str: return (tree if print_tree else format_term)(self, {})
  def __repr__(self)->str: return str(self)

def x(var:int) -> Node: return Node(Tag.intermediate_var, label = var)

def num(n:int) -> Node: return Node(Tag.Prim, label = n)

def var(lam:Node) -> Node:
  lam.s1 = Node(Tag.Var, lam)
  return lam.s1

def app(func:Node, arg:Node) -> Node: return Node(Tag.App, func, arg)

ilab = 70

def sup(a:Node, b:Node, label:int = None)->Node:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  return Node(Tag.Sup, a, b, label)

def dup(s0:Node, label:int = None)->tuple[Node, Node]:
  global ilab
  if label is None: label = (ilab := ilab + 1)
  d = Node(Tag.Dup, s0, label = label)
  d2 = Node(Tag.Dup2, s0, d, label)
  d.s1 = d2
  d2.s1 = d
  return d, d2

def null(): return Node(Tag.Null)


def lam(body:Node) -> Node:
  res = Node(Tag.Lam, body)
  parse_lam(res, body, 0)
  return res

def move(src:Node, dst:Node )->Node:
  dst.tag = src.tag
  dst.s0 = src.s0
  dst.s1 = src.s1
  dst.label = src.label
  if src.tag == Tag.Var:
    lam = src.s0
    assert lam.tag == Tag.Lam
    lam.s1 = dst
  if src.tag == Tag.Lam:
    dst.s1.s0 = dst
  if src.tag in [Tag.Dup, Tag.Dup2]:
    dst.s1.s1 =dst
  return dst

def parse_lam(lam:Node, current:Node, depth:int)->Node:
  match current.tag:
    case Tag.Lam:
      parse_lam(lam, current.s0, depth + 1)
    case Tag.intermediate_var if current.label == depth:
      if (lam.s1):
        a,b = dup(Node(Tag.Var, lam))
        move(a, lam.s1)
        move(b, current)
        lam.s1 = a.s0
      else:
        lam.s1 = move(Node(Tag.Var, lam), current)
    case Tag.App | Tag.Sup:
      parse_lam(lam, current.s0, depth)
      parse_lam(lam, current.s1, depth)
    case Tag.Dup | Tag.Dup2:
      parse_lam(lam, current.s0, depth)

def format_term(term:Node, ctx: dict[Node, int])->str:
  def varname(node:Node):
    if node not in ctx: ctx[node] = chr(len(ctx) + 96)
    return ctx[node]
  ctx[None] = ""
  match term.tag:
    case Tag.Lam: return f"λ{varname(term.s1)}.{format_term(term.s0, ctx)}"
    case Tag.App: return f"({format_term(term.s0, ctx)} {format_term(term.s1, ctx)})"
    case Tag.Var: return varname(term)
    case Tag.Dup | Tag.Dup2:
      
      if hide_dups: return format_term(term.s0, ctx)
      d1 = term if (term.tag == Tag.Dup) else term.s1
      if d1 in ctx: return varname(d1 if term == d1 else d1.s1)
      a = varname(d1)
      b = varname(d1.s1)
      return f"&{{{a},{b}}} = {format_term(term.s0,ctx)}; {a if term==d1 else b}"
    case Tag.Sup: return f"&{{{format_term(term.s0, ctx)}, {format_term(term.s1, ctx)}}}"
    case Tag.Null: return "Nul"
    case Tag.Prim: return str(term.label)
  return str(term.tag)

def tree(term:Node, ctx:dict[Node, int])->str:
  def varname(node:Node | None):
    if node is None: return ""
    return ctx.setdefault(node, chr(len(ctx) + 97))
  def idn(lns:list[str])->list[str]:
    if sum(len(ln) for ln in lns) <= 20: return ["  " + " ".join(map(str.strip, lns))]
    return ["  " + ln for ln in lns]
  def prep(head:str, lns:list[str])->list[str]: return [head + " " + lns[0].strip()] + lns[1:]
  def _tree(term:Node, dstack:list[tuple[int, bool]])->list[str]:
    if term is None: return ["NONE"]
    match term.tag:
      case Tag.Lam: return prep(f"λ{varname(term.s1)}", _tree(term.s0, dstack))
      case Tag.Sup:
        for i, (label, is_dup2) in reversed(list(enumerate(dstack))):
          if term.label == label:
            stack = dstack[:i] + dstack[i+1:]
            return _tree(term.s1 if is_dup2 else term.s0, stack)
        return [term.tag.name + (f"{term.label}")] + idn(_tree(term.s0, dstack)) + idn(_tree(term.s1, dstack))
      case Tag.App:
        return ["app"] + idn(_tree(term.s0, dstack)) + idn(_tree(term.s1, dstack))
      case Tag.Dup | Tag.Dup2:
        if hide_dups: return _tree(term.s0, dstack + ([(term.label, term.tag == Tag.Dup2)]))

        d1 = term if (term.tag == Tag.Dup) else term.s1
        if d1 in ctx: return [varname(term)]

        return [f"{term.label}{{{varname(d1)}, {varname(d1.s1)}}} ="] + idn(_tree(term.s0, dstack)) + idn([f"in {varname(term)}"])

      case Tag.Prim: return [str(term.label)]
      case Tag.Null: return ["Nul"]
    return [varname(term)]
  return "\n".join(_tree(term, []))
