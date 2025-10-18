import subprocess, ctypes, tempfile, os
from node import Node, lam, print_tree, x, app, sup, dup, var, null, Tag, hide_dups, tree
from run import step, move
from typing import Callable


def to_c_data(node: Node) -> list[int]:
  """Serialize a Node graph to a flat int array for passing to C"""
  ctx = {}
  nodes = []
  
  def visit(n: Node):
    if n is None or n in ctx:
      return
    ctx[n] = len(nodes) + 1  # 1-indexed, 0 is NULL
    nodes.append(n)
    visit(n.s0)
    visit(n.s1)
  
  visit(node)
  
  # Tag enum mapping to match C enum order
  tag_map = {
    Tag.App: 0,
    Tag.Lam: 1, 
    Tag.Sup: 2,
    Tag.Dup: 3,
    Tag.Dup2: 4,
    Tag.Null: 5,
    Tag.Var: 6,
  }
  
  # Format: [count, tag1, label1, s0_idx1, s1_idx1, tag2, ...]
  data = [len(nodes)]
  for n in nodes:
    tag_int = tag_map.get(n.tag, 0)
    s0_idx = ctx.get(n.s0, 0)
    s1_idx = ctx.get(n.s1, 0)
    data.extend([tag_int, n.label or 0, s0_idx, s1_idx])
  
  return data



def from_c_data(res:ctypes.POINTER(ctypes.c_int))->Node:
  l = res[0]
  
  # Check for error condition (segfault in C code)
  if l == -1:
    raise RuntimeError("Segmentation fault occurred in C code")
  
  nodes = [None] + [Node(None) for _ in range(l)]
  for i in range(l):

    tag = [Tag.App, Tag.Lam, Tag.Sup, Tag.Dup, Tag.Dup2, Tag.Null, Tag.Var][res[i * 4 + 1]]
    nodes[i + 1].tag = tag
    nodes[i + 1].label = res[i * 4 + 2]
    nodes[i + 1].s0 = nodes[res[i * 4 + 3]]
    nodes[i + 1].s1 = nodes[res[i * 4 + 4]]

  return nodes[1]



os.makedirs("./.tmp", exist_ok=True)

c_path = os.path.join("./.tmp", "main.c")
so_path = os.path.join("./.tmp", "main.so")

with open("main.c") as src:
  code_template = src.read()

subprocess.check_call(["clang", "-shared", "-O2", "-fPIC", "main.c", "-o", so_path])
lib = ctypes.CDLL(so_path)
lib.work.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
lib.work.restype = ctypes.POINTER(ctypes.c_int)

def run_term_c(term: Node, steps: int = 100) -> Node:
  try:
    graph_data = to_c_data(term)
    res = lib.work((ctypes.c_int * len(graph_data))(*graph_data), steps)
    return from_c_data(res)
  except RuntimeError as e:
    print(f"Error: {e}")
    return term



def c2():
  return lam(lam(app(x(1), app(x(1), x(0)))))

def id():
  return lam(x(0))

def circular1():
  temp = null()
  dups = dup(sup(temp, null(), 0), 0)
  move(dups[0], temp)
  return dups[1]


def circular2():
  temp = null()
  dups = dup(sup(temp, null(), 0), 0)
  move(dups[1], temp)
  return dups[0]


def circular3():
  temp = null()
  dups = dup(sup(null(), temp , 0), 0)
  move(dups[1], temp)
  return dups[0]


def circular4():
  temp = null()
  dups = dup(sup(null(), temp , 0), 0)
  move(dups[0], temp)
  return dups[1]

  

node = circular4()


print_tree.set(False)

print(node)

print(node)

node = run_term_c(node, 100)

print(node)

hide_dups.set(True)
print(node)