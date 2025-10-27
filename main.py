import subprocess, ctypes, tempfile, os, hashlib
from example import cnat
from node import DEBUG, Node, lam, print_tree, x, app, sup, dup, var, null, Tag, hide_dups, tree
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
  
  if l == -1:
    raise RuntimeError("Segmentation fault occurred in C code")
  
  nodes = [None] + [Node(None) for _ in range(l)]
  for i in range(l):


    tag = [Tag.App, Tag.Lam, Tag.Sup, Tag.Dup, Tag.Dup2, Tag.Null, Tag.Var][res[i * 4 + 1]]
    # print(i + 1, tag, res[i * 4 + 2], res[i * 4 + 3], res[i * 4 + 4])
    nodes[i + 1].tag = tag
    nodes[i + 1].label = res[i * 4 + 2]
    nodes[i + 1].s0 = nodes[res[i * 4 + 3]]
    nodes[i + 1].s1 = nodes[res[i * 4 + 4]]

  return nodes[1]


so_path = os.path.join("./.tmp", "main.so")
c_cache_path = os.path.join("./.tmp", "c_hash")


with open("main.c", "rb") as f: c_hash = hashlib.md5(f.read()).hexdigest()

def compile_c():
  with open(c_cache_path, "w") as f:
    f.write(c_hash)
  print("Compiling...")
  os.makedirs("./.tmp", exist_ok=True)
  subprocess.check_call(["clang", "-shared", "-O2", "-fPIC", "main.c", "-o", so_path])

if not os.path.exists(c_cache_path): compile_c()
else:
  with open(c_cache_path, "r") as f:
    if  f.read().strip() != c_hash: compile_c()



lib = ctypes.CDLL(so_path)
lib.load.argtypes = [ctypes.POINTER(ctypes.c_int)]
lib.load.restype = ctypes.c_int
lib.unload.argtypes = []
lib.unload.restype = ctypes.POINTER(ctypes.c_int)
lib.set_debug.argtypes = [ctypes.c_int]

lib.run.argtypes = [ctypes.c_int]
lib.run.restype = ctypes.c_int

lib.set_debug(DEBUG.get())

def load_term_c(term: Node) -> Node: return lib.load((ctypes.c_int * len(to_c_data(term)))(*to_c_data(term)))

def unload_term_c() -> Node: return from_c_data(lib.unload())

def run(steps:int): return lib.run(steps)

def run_term_c(term:Node, steps: int = 1e6) -> Node:
  print("running term")
  load_term_c(term)
  steps = run(steps)
  res = unload_term_c()
  print("ran term")
  return res



if __name__ == "__main__":
  term = cnat(2)
  print(term)
  load_term_c(term)
  run(100)
  res = unload_term_c()
  print(res)

  term = cnat(2)
  print(term)
  load_term_c(term)
  run(100)
  res = unload_term_c()
  print(res)

  






