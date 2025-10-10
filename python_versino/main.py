
#%%

from enum import Enum
from dataclasses import dataclass



# class PortType(Enum):
#   main = 0
#   aux1 = 1
#   aux2 = 2
#   def __str__(self) -> str:
#     return f"{self.name}"

# MAIN = PortType.main
# AUX1 = PortType.aux1
# AUX2 = PortType.aux2

# class NodeType(Enum):
#   era = 0
#   null = 1
#   lam = 2
#   app = 3
#   dup = 4
#   sup = 5
#   root = 6
#   intermediate_var = 7
#   def __str__(self)->str:
#     return self.name

# ERA = NodeType.era
# NULL = NodeType.null
# LAM = NodeType.lam
# APP = NodeType.app
# DUP = NodeType.dup
# SUP = NodeType.sup
# ROOT = NodeType.root

  
# @dataclass
# class Port:
#   node: "Node"
#   port: PortType

#   def polarity(self)->bool:
#     match self.node.nodeType:
#       case NodeType.era | NodeType.root:
#         return True
#       case NodeType.null | NodeType.intermediate_var:
#         return False
#       case NodeType.lam:
#         return True if self.port == PortType.aux2 else False
#       case NodeType.app:
#         return False if self.port == PortType.aux2 else True
#       case NodeType.dup:
#         return True if self.port == PortType.main else False
#       case NodeType.sup:
#         return False if self.port == PortType.main else True
  
#   # def other(self)->"Port":
#   #   return self.node.get_port(self.port_type)
  
#   @property
#   def Main(self)->"Node":
#     return self.node.Main
#   @property
#   def Aux1(self)->"Node":
#     return self.node.Aux1
#   @property
#   def Aux2(self)->"Node":
#     return self.node.Aux2
#   @property
#   def main(self)->"Node":
#     return self.node.main
#   @property
#   def aux1(self)->"Node":
#     return self.node.aux1
#   @property
#   def aux2(self)->"Node":
#     return self.node.aux2
  
#   def __repr__(self):
#     return f"<{self.node.nodeType} {self.port}>"


  
# def polarity(node: NodeType, port: PortType)->bool:
#   match node:
#     case NodeType.era | NodeType.root:
#       return True
#     case NodeType.null | NodeType.intermediate_var:
#       return False
#     case NodeType.lam:
#       return True if port == PortType.aux2 else False
#     case NodeType.app:
#       return not polarity(NodeType.lam, port)
#     case NodeType.dup:
#       return True if port == PortType.main else False
#     case NodeType.sup:
#       return not polarity(NodeType.dup, port)

class Node:

  def __init__(self):
    pass

  def __str__(self):
    return format_term(self, {})
  
  def __repr__(self):
    return format_term(self, {})

class Null(Node):
  pass

class Lam(Node):
  def __init__(self, body:Node, var:None):
    self.body = body
    self.var = var
    parse_lam(self, body, 0)

class Var(Node):
  def __init__(self, lam:Lam | int):
    self.lam = lam

class App(Node):
  def __init__(self, func:Node, arg:Node):
    self.func = func
    self.arg = arg

class Dup1(Node):
  def __init__(self, target:Node, label:int):
    self.target = target
    self.label = label

class Dup2(Node):
  def __init__(self, target:Dup1):
    self.target = target

class Sup(Node):
  def __init__(self, a:Node, b:Node, label:int):
    self.a = a
    self.b = b
    self.label = label


def parse_lam(lam:Lam, current:Node, depth:int)->Node:
  match current:
    case Lam():
      parse_lam(lam, current.body, depth + 1)
    case App():
      parse_lam(lam, current.func, depth)
      parse_lam(lam, current.arg, depth)
    case Var():
      if current.lam == depth:
        current.lam = lam
    case Dup1() | Dup2():
      parse_lam(lam, current.target, depth)
    case Sup():
      parse_lam(lam, current.a, depth)
      parse_lam(lam, current.b, depth)



#%%



def format_term(term:Node, ctx: dict[Lam, int])->str:
  match term:
    case Lam():
      if not term in ctx: ctx[term] = len(ctx)
      return f"λ{chr(ctx[term] + 97)}.{format_term(term.body, ctx)}"
    case App():
      return f"({format_term(term.func, ctx)} {format_term(term.arg, ctx)})"
    case Var():
      if not term.lam in ctx: ctx[term.lam] = len(ctx)
      return f"{chr(ctx[term.lam] + 97)}"
    case Dup1() | Dup2():
      return format_term(term.target, ctx)
    case Sup():
      return f"&{{{format_term(term.a, ctx)} {format_term(term.b, ctx)}}}"
    case Null():
      return "Nul"
  return "UNK"


#%%


def reduce(term:Node):
  match term:
    case Lam():
      reduce(term.body)
    case App():
      if term.func == 


#%%
