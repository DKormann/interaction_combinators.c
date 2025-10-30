import subprocess, ctypes, os, hashlib, threading
from example import cnat
from node import DEBUG, Node, Tag, hide_dups



def to_c_data(node: Node) -> list[int]:
  """Serialize a Node graph to a flat int array for passing to C"""
  ctx = {}
  nodes = []

  taken = {}
  
  def take(owner: Node, owned: Node):
    if owned in taken:
      raise ValueError(f"Node {owned} is already taken by {taken[owned]}")
    taken[owned] = owner
  
  def visit(n: Node):
    if n is None or n in ctx:
      return
    ctx[n] = len(nodes) + 1  # 1-indexed, 0 is NULL
    nodes.append(n)
    visit(n.s0)
    visit(n.s1)

    match n.tag:
      case Tag.App:
        take(n, n.s0)
        take(n, n.s1)
      case Tag.Lam:
        take(n, n.s0)
      case Tag.Sup:
        take(n, n.s0)
        take(n, n.s1)

  
  visit(node)

  for i, n in enumerate(nodes):
    if n.tag == Tag.App:
      if n.s1 is None:
        raise ValueError(f"Invalid App at index {i}: s1 (argument) is None. App must have both function (s0) and argument (s1). s0={n.s0}")
  
  taken = set[int]()

  def take(n:int):
    if n and n in taken:
      raise ValueError(f"Node {n} is already taken")
    taken.add(n)

  
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
  
  data = [len(nodes)]
  for i, n in enumerate(nodes):
    tag_int = tag_map.get(n.tag, 0)
    s0_idx = ctx.get(n.s0, 0)
    s1_idx = ctx.get(n.s1, 0)

    if DEBUG: print(f"{i}: to_c_data: {n.tag} {s0_idx} {s1_idx}")


    match n.tag:
      case Tag.App:
        take(s0_idx)
        take(s1_idx)
      case Tag.Lam:
        take(s0_idx)
      case Tag.Sup:
        take(s0_idx)
        take(s1_idx)
      case Tag.Dup:
        take(s0_idx)



    data.extend([tag_int, n.label or 0, s0_idx, s1_idx])


  
  return data

def from_c_data(res:ctypes.POINTER(ctypes.c_int))->Node:

  l = res[0]
  if l == -1: raise RuntimeError("Segmentation fault occurred in C code")
  
  nodes = [None] + [Node(None) for _ in range(l)]
  for i in range(l):
    tag = [Tag.App, Tag.Lam, Tag.Sup, Tag.Dup, Tag.Dup2, Tag.Null, Tag.Var][res[i * 4 + 1]]
    nodes[i + 1].tag = tag
    nodes[i + 1].label = res[i * 4 + 2]
    nodes[i + 1].s0 = nodes[res[i * 4 + 3]]
    nodes[i + 1].s1 = nodes[res[i * 4 + 4]]

  return nodes[1]


so_path = os.path.join("./.tmp", "main.so")
c_cache_path = os.path.join("./.tmp", "c_hash")
main_path = os.path.join(os.path.dirname(__file__), "main.c")
main_path = os.path.join(os.path.dirname(__file__), "mini.c")


with open(main_path, "rb") as f: c_hash = hashlib.md5(f.read()).hexdigest()

def compile_c():
  with open(c_cache_path, "w") as f:
    f.write(c_hash)
  print("Compiling...")
  os.makedirs("./.tmp", exist_ok=True)
  subprocess.check_call(["clang", "-shared", "-O2", "-fPIC", main_path, "-o", so_path])

if not os.path.exists(c_cache_path): compile_c()
else:
  with open(c_cache_path, "r") as f:
    if  f.read().strip() != c_hash: compile_c()


_thread_local = threading.local()

def get_lib():
  if not hasattr(_thread_local, 'lib'):
    _thread_local.lib = ctypes.CDLL(so_path)
    _thread_local.lib.load.argtypes = [ctypes.POINTER(ctypes.c_int)]

    _thread_local.lib.unload.argtypes = []
    _thread_local.lib.unload.restype = ctypes.POINTER(ctypes.c_int)
    _thread_local.lib.set_debug.argtypes = [ctypes.c_int]
    _thread_local.lib.run.argtypes = [ctypes.c_int]
    _thread_local.lib.run.restype = ctypes.c_int
    _thread_local.lib.set_debug(DEBUG.get())
  return _thread_local.lib

def load_term_c(term: Node) -> None:
  lib = get_lib()

  lib.load((ctypes.c_int * len(to_c_data(term)))(*to_c_data(term)))

def unload_term_c() -> Node:
  lib = get_lib()
  res = from_c_data(lib.unload())
  return res

def run(steps:int = 1e6):
  lib = get_lib()
  return lib.run(int(steps))

def run_term_c(term:Node, steps: int = 1e6) -> Node:
  load_term_c(term)
  steps = run(int(steps))
  res = unload_term_c()
  return res

# if __name__ == "__main__":
#   term = cnat(2)(cnat(2))
#   print(term)
#   load_term_c(term)
#   run(100)
#   res = unload_term_c()

#   print(res)
#   with hide_dups(True):
#     print(res)

