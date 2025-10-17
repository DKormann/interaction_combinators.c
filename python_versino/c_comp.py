import subprocess, ctypes, tempfile, os
from node import Node, lam, x, app, sup, dup, var, null, Tag, hide_dups, print_tree, tree
from run import step


def to_c(node:Node)->str:

  ctx = {}

  def go(node:Node, code:str)->str:
    if node is None: return code
    if node in ctx: return code

    myname = f"x{len(ctx)}"
    ctx[node] = myname

    code += f'Node* {myname} = malloc(sizeof(Node));\n'
    code = go(node.s0, code)
    code = go(node.s1, code)
    label = node.label if node.label is not None else 0
    code += f'{myname}->tag = Tag_{node.tag};\n'
    code += f'{myname}->label = {label};\n'

    if node.s0 is not None: code += f'{myname}->s0 = {ctx[node.s0]};\n'
    if node.s1 is not None: code += f'{myname}->s1 = {ctx[node.s1]};\n'
    return code

  nodes = go(node, "")

  return f'''Node* setup(void){{
{nodes}
return {ctx[node]};
}}
'''



def from_c(res:ctypes.POINTER(ctypes.c_int))->Node:
  l = res[0]
  nodes = [None] + [Node(None) for _ in range(l)]
  for i in range(l):

    tag = [Tag.App, Tag.Lam, Tag.Sup, Tag.Dup, Tag.Dup2, Tag.Null, Tag.Var][res[i * 4 + 1]]
    nodes[i + 1].tag = tag
    nodes[i + 1].label = res[i * 4 + 2]
    nodes[i + 1].s0 = nodes[res[i * 4 + 3]]
    nodes[i + 1].s1 = nodes[res[i * 4 + 4]]

  return nodes[1]


import time

with open("main.c") as src: code_template = src.read()


st = time.time_ns()

print(f"open files time: {time.time_ns() - st}")
os.makedirs("./.tmp", exist_ok=True)

c_path = os.path.join("./.tmp", "main.c")
so_path = os.path.join("./.tmp", "main.so")
c_file = open(c_path, "w")

print(f"file open done: {time.time_ns() - st}")


from typing import Callable

def get_worker(term:Node)->Callable[[int], Node]:
  global code_template
  code_template = code_template.replace("/*SETUP*/", to_c(term))

  c_file.seek(0)
  c_file.write(code_template)
  c_file.truncate()
  c_file.flush()

  code_template = code_template.replace("/*SETUP*/", "int c = 22;")

  subprocess.check_call(["clang", "-shared", "-O2", "-fPIC", c_path, "-o", so_path])

  lib = ctypes.CDLL(so_path)
  lib.work.argtypes = [ctypes.c_int]
  lib.work.restype = ctypes.POINTER(ctypes.c_int)

  def worker(steps: int)->Node:
    res = lib.work(steps)
    return from_c(res)

  print(f"worker created: {time.time_ns()-st}")

  return worker

last_worker = None



def A(): return app(lam(sup(x(0), app(x(0), null()))), lam(x(0)))

worker = get_worker(A())

for i in range(3):
  l = A()
  node = worker(4)

  print(f"don: {time.time_ns() - st}")

  hide_dups.set(False)
  print(node)


c_file.close()