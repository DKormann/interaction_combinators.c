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

c = Y_comb()(copyn())(nat(1))

# hide_dups.set(True)
print(c)

c = run_term_c(c)


print(c)

print(sup(null(), null()))


