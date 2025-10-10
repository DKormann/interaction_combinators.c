

#%%

from enum import Enum

class NodeType(Enum):
  era = 0
  null = 1
  lam = 2
  app = 3
  dup = 4
  sup = 5
  root = 6
  var = 7
  intermediate_var = 8
  def __str__(self)->str:
    return self.name

ERA = NodeType.era
NULL = NodeType.null
LAM = NodeType.lam
APP = NodeType.app
DUP = NodeType.dup
SUP = NodeType.sup
VAR = NodeType.var
ROOT = NodeType.root


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


#%%


class Node:
  def __init__(self, nodeType:NodeType):
    self.nodeType = nodeType
    self.target = None

