from base64 import decodebytes
from itertools import count
import subprocess, ctypes, os, hashlib, threading
from example import cnat
from node import DEBUG, Node, Tag, hide_dups



def to_c_data(node: Node) -> list[int]:
  """Serialize a Node graph to a flat int array for passing to C"""
  ctx = {}
  nodes = []
  taken = {}

  def take(owner: Node, owned: Node):
    if owned in taken: raise ValueError(f"Node {owned} is already taken by {taken[owned]}")
    taken[owned] = owner
  
  def visit(n: Node):
    if n is None or n in ctx:
      return
    ctx[n] = len(nodes) + 1  # 1-indexed, 0 is NULL
    nodes.append(n)
    visit(n.s0)
    visit(n.s1)

    if DEBUG:
      match n.tag:
        case Tag.Lam: take(n, n.s0)
        case Tag.App | Tag.Sup:
          take(n, n.s0)
          take(n, n.s1)

  visit(node)

  for i, n in enumerate(nodes):
    if n.tag == Tag.App:
      if n.s1 is None:
        raise ValueError(f"Invalid App at index {i}: s1 (argument) is None. App must have both function (s0) and argument (s1). s0={n.s0}")
  
  # Tag enum mapping to match C enum order
  tag_map = {
    Tag.App: 0,
    Tag.Lam: 1, 
    Tag.Sup: 2,
    Tag.Dup: 3,
    Tag.Dup2: 4,
    Tag.Null: 5,
    Tag.Var: 6,
    Tag.Freed: 7,
  }
  
  data = [len(nodes)]
  for i, n in enumerate(nodes):
    tag_int = tag_map.get(n.tag, 0)
    s0_idx = ctx.get(n.s0, 0)
    s1_idx = ctx.get(n.s1, 0)

    data.extend([tag_int, n.label or 0, s0_idx, s1_idx])
  
  return data

def from_c_data(res:ctypes.POINTER(ctypes.c_int))->Node:


  tags = [Tag.App, Tag.Lam, Tag.Sup, Tag.Dup, Tag.Dup2, Tag.Null, Tag.Var, Tag.Freed]

  l = res[0]
  if l == -1: raise RuntimeError("Segmentation fault occurred in C code")
  
  nodes = [None] + [Node(None) for _ in range(l)]

  for i in range(l):
    if DEBUG>1: print(res[i * 4 + 1])
    nodes[i + 1].tag = tags[res[i * 4 + 1]]
    nodes[i + 1].label = res[i * 4 + 2]
    nodes[i + 1].s0 = nodes[res[i * 4 + 3]]
    nodes[i + 1].s1 = nodes[res[i * 4 + 4]]

  def refcount(node:Node)->int: return sum((n.s0 == node) + (n.s1 == node) for n in nodes[1:])

  for node in nodes[1:]:
    if node.tag == Tag.Lam and refcount(node.s1) < 2: node.s1 = None

  return nodes[1]


so_path = os.path.join("./.tmp", "main.so")
c_cache_path = os.path.join("./.tmp", "c_hash")
main_path = os.path.join(os.path.dirname(__file__), "main.c")


with open(main_path, "rb") as f: c_hash = hashlib.md5(f.read()).hexdigest()

def compile_c():
  with open(c_cache_path, "w") as f: f.write(c_hash)
  print("Compiling...")
  os.makedirs("./.tmp", exist_ok=True)
  subprocess.check_call(["clang", "-shared", "-O2", "-fPIC", main_path, "-o", so_path])

if not os.path.exists(c_cache_path): compile_c()
else:
  with open(c_cache_path, "r") as f:
    if  f.read().strip() != c_hash: compile_c()


_local = threading.local()

def get_lib():
  if not hasattr(_local, 'lib'):
    _local.lib = ctypes.CDLL(so_path)
    _local.lib.load.argtypes = [ctypes.POINTER(ctypes.c_int)]

    _local.lib.unload.argtypes = []
    _local.lib.unload.restype = ctypes.POINTER(ctypes.c_int)
    _local.lib.set_debug.argtypes = [ctypes.c_int]
    _local.lib.run.argtypes = [ctypes.c_int]
    _local.lib.run.restype = ctypes.c_int
    _local.lib.set_debug(DEBUG.get())
  return _local.lib

def load_term_c(term: Node) -> None:
  lib = get_lib()

  lib.load((ctypes.c_int * len(to_c_data(term)))(*to_c_data(term)))

def unload_term_c() -> Node:
  lib = get_lib()
  res = from_c_data(lib.unload())
  return res


DEFAULT_FUEL = 1<<30

def run(steps:int = DEFAULT_FUEL): return get_lib().run(steps)

def run_term_c(term:Node, steps: int = DEFAULT_FUEL) -> Node:
  load_term_c(term)
  steps = run(int(steps))
  res = unload_term_c()
  return res
