"""
Scott encoding for natural numbers and functions on them
"""

from example import F, T
from main import run_term_c
from node import Node, Tag, app, hide_dups, lam, move, null, parse_lam, print_tree, sup, x, dup

def nat(n:int)->Node:
  if n == 0: return Node(lambda s, z: z)
  return Node(lambda s, z: s(nat(n-1)))



def Y_comb()->Node:
  return Node(lambda f: Node(lambda x: f(x(x))) (Node(lambda x: f(x(x)))))



def is_z()->Node:
  return Node(lambda n: n(
    lambda s: F(),
    T()
  ))

# print(is_z())



def copyn()->Node:
  return Node(
    lambda self, n: n(
      lambda s: nat(1),
      nat(0)
    )
  )

# print(Y_comb())

# c = Y_comb()(copyn())(nat(1))

# print(c)

def linear_check(term:Node):
  c = {}


  def toggle(term:Node):
    if term is None: return

    if term in c: del c[term]
    else: c[term] = True
  
  def go(term:Node):
    match term.tag:
      case Tag.Dup: toggle(term)
      case Tag.Dup2:
        toggle(term.s1)
        return
      case Tag.Lam: toggle(term.s1)
      case Tag.Var: toggle(term)
    
    for src in term.srcs(): go(src)
  
  go(term)

  if len(c) == 0: return True
  for v in c:
    print('-'*10, 'linear check FAIL')
    print(v)
    print(v.s0)
  print(c)
  





c = Y_comb()(copyn())(nat(1))

for i in range(10):
  c = run_term_c(c,1)
  print(c)
  if not linear_check(c): break
  print('-'*10, 'linear check passed')

# print(c)

