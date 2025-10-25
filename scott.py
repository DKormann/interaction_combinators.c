"""
Scott encoding for natural numbers and functions on them
"""

from operator import truediv
from example import F, T
from main import run_term_c
from node import Node, Tag, app, hide_dups, lam, move, null, parse_lam, print_tree, sup, x, dup

def nat(n:int)->Node:
  if n == 0: return Node(lambda s, z: z)
  return Node(lambda s, z: s(nat(n-1)))


def iden()->Node:
  return Node(lambda x: x)


def Y_comb()->Node:
  return Node(lambda f: Node(lambda x: f(x(x))) (Node(lambda x: f(x(x)))))


def is_z()->Node:
  return Node(lambda n: n(
    lambda s: F(),
    T()
  ))

def rec0()->Node:
  return Node(
    lambda self, n: n(
      lambda s: self(s),
      nat(0)
    )
  )


def suc(x:Node)->Node:
  return Node( lambda s,z: s(x))

def rec_copy()->Node:
  return Node(
    lambda self, x: x(
      lambda s: suc(self(s)),
      nat(0)
    )
  )

from example import cnat

if __name__ == "__main__":


  c = cnat(2)(cnat(2))

  prev = str(c)
  for i in range(1):
    print('-'*10, 'step', i)
    c = run_term_c(c, 30)
    hide_dups.set(False)
    print(c)
    s = str(c)
    if s == prev: break
    prev = s
  
  with hide_dups.context(True):
    print(c)
