
from contextlib import contextmanager
from enum import Enum, auto
from string import Formatter
from typing import Callable

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
    self.value = value
    if name in os.environ: self.value = int(os.environ[name])
    else: self.value = value
  
  def set(self, value:int): self.value = value
  def get(self)->int: return self.value
  def __str__(self)->str: return f"{self.value}"
  def __bool__(self)->bool: return bool(self.value)
  def __int__(self)->int: return int(self.value)
  def __lt__(self, other)->bool: return self.value < int(other)
  def __eq__(self, other)->bool: return self.value == int(other)

  @contextmanager
  def __call__(self, value: int):
    old_value = self.value
    self.value = value
    try: yield
    finally: self.value = old_value


hide_dups = Env("hide_dups", False)
print_tree = Env("tree", True)
DEBUG = Env("DEBUG", 0)

class Node:
  def __init__(self, tag: Tag | None | Callable | int = None, s0:"Node" = None, s1:"Node" = None, label:int = None):
    if tag is None: tag = Tag.Null
    self.s0 = None if s0 is None else Node.into(s0)
    self.s1 = None if s1 is None else Node.into(s1)
    self.label = label
    if isinstance(tag, Tag): self.tag = tag
    else: move(Node.into(tag),self)
  def __str__(self)->str: return tree(self, {})
  def __repr__(self)->str: return str(self)
  def dup(self, label:int = None)->"Node":
    ds = dup(move(self), label)
    move(ds[0], self)
    return ds[1]
  def srcs(self):
    res = []
    if self.tag in [Tag.Lam, Tag.Dup, Tag.Dup2, Tag.App, Tag.Sup]: res.append(self.s0)
    if self.tag in [Tag.App, Tag.Sup]: res.append(self.s1)
    return res


  @staticmethod
  def into(arg)->"Node":
    if isinstance(arg, Node): return arg
    if callable(arg):

      l = Node(Tag.Lam, null(), None)
      v = Node(Tag.Var, l)
      l.s1 = v
      l.s0 = Node.into(mk_curried(arg)(v))

      found = False
      def arg_dups(arg:Node):
        nonlocal found
        if arg == v:
          if found: return v.dup()
          found = True
        if arg.tag in [Tag.Dup, Tag.Dup2, Tag.App, Tag.Sup, Tag.Lam]: arg.s0 = arg_dups(arg.s0)
        if arg.tag in [Tag.App, Tag.Sup]: arg.s1 = arg_dups(arg.s1)
        return arg

      arg_dups(l.s0)
      if not found: l.s1 = None
      return l
    if (isinstance(arg, int)): return Node(Tag.Prim, label = arg)
    if arg is None: return Node(Tag.Null)
  
  def __call__(self, *args:"Node")->"Node":
    if not args: return self
    return app(self, args[0])(*args[1:])



def mk_curried(arg:Callable)->Callable:
  args = []
  def curried(x:Node)->Node:
    args.append(x)
    if len(args) == arg.__code__.co_argcount: return arg(*args)
    return curried
  return curried





def x(var:int) -> Node: return Node(Tag.intermediate_var, label = var)

def num(n:int) -> Node: return Node(Tag.Prim, label = n)

def var(lam:Node) -> Node:
  lam.s1 = Node(Tag.Var, lam)
  lam.s1.s0 = lam
  return lam.s1

def app(func:Node, arg:Node) -> Node: return Node(Tag.App, func, arg)

ilab = 70

def reset_labels():
  global ilab
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

def move(src:Node, dst:Node | None = None)->Node:
  if dst is None: dst = Node(None)
  if isinstance(src, list) or isinstance(src, tuple): return [move(s, d) for s, d in zip(src, dst)]

  dst.tag = src.tag
  dst.s0 = src.s0
  dst.s1 = src.s1
  dst.label = src.label
  if src.tag == Tag.Var:
    lam = src.s0
    assert lam.tag == Tag.Lam
    lam.s1 = dst
  if src.tag == Tag.Lam and src.s1 is not None:
    dst.s1.s0 = dst
  if src.tag in [Tag.Dup, Tag.Dup2]:
    dst.s1.s1 =dst
  return dst



def parse_lam(lam:Node, current:Node, depth:int):
  print("parse_lam", current.tag)
  if current.tag == Tag.Lam: parse_lam(lam, current.s0, depth + 1)
  elif current.tag in [Tag.App, Tag.Sup]: parse_lam(lam, current.s1, depth)
  elif current.tag in [Tag.Dup, Tag.Dup2, Tag.App, Tag.Sup]: parse_lam(lam, current.s0, depth)
  elif ((current.tag == Tag.intermediate_var and current.label == depth) or
        (current.tag == Tag.Var and current.s0 == lam and lam.s1 is not current)):
    if lam.s1 is None: move(var(lam), current)
    else:
      prev = lam.s1
      move(dup(var(lam)), [prev,current])



def tree(term:Node, ctx:dict[Node, int])->str:
  ws = "  " if print_tree else ""
  def varname(node:Node | None):
    if node is None: return ""
    return ctx.setdefault(node, chr(len(ctx) + 97))
  def idn(lns:list[str], end = "")->list[str]:
    lns = lns[:-1] + [lns[-1] + end]
    if sum(len(ln) for ln in lns) <= 20: return [ws + " ".join(map(str.strip, lns))]
    return [ws + ln for ln in lns]

  def _tree(term:Node | None, dstack:list[tuple[int, bool]])->list[str]:
    if term is None: return ["NONE"]
    match term.tag:
      case Tag.Lam: return [f"λ{varname(term.s1)} " + (p := _tree(term.s0, dstack))[0].strip()] + p[1:]
      case Tag.Sup:
        for i, (label, is_dup2) in reversed(list(enumerate(dstack))):
          if term.label == label: return _tree(term.s1 if is_dup2 else term.s0, dstack[:i] + dstack[i+1:])
        return [f"&{term.label}{{"] + idn(_tree(term.s0, dstack)) + idn(_tree(term.s1, dstack), "}")
        
      case Tag.App: return [f"("] + idn(_tree(term.s0, dstack)) + idn(_tree(term.s1, dstack), ")")
      case Tag.Dup | Tag.Dup2:
        if hide_dups: return _tree(term.s0, dstack + ([(term.label, term.tag == Tag.Dup2)]))

        d1 = term if (term.tag == Tag.Dup) else term.s1
        d2 = term if (term.tag == Tag.Dup2) else term.s1
        if d1 in ctx: return [varname(term)]
        # return [f"&{term.label}{{{varname(d1)}, {varname(d2)}}} ="] + idn(_tree(term.s0, dstack)) + idn([f"in {varname(term)}"])
        return [f"{varname(term)} where &{term.label}{{{varname(d1)}, {varname(d2)}}} ="] + idn(_tree(term.s0, dstack))

      case Tag.Prim: return [str(term.label)]
      case Tag.Null: return ["Nul"]
    return [varname(term)]
  return ("\n" if print_tree else " ").join(_tree(term, []))
