"""
Scott encoding for natural numbers and functions on them
"""

from operator import truediv
from typing_extensions import runtime
from example import F, T
from main import load_term_c, run, run_term_c, unload_term_c
from node import DEBUG, Node, Tag, app, hide_dups, lam, lamvar, move, null, parse_lam, print_tree, sup, x, dup

from example import cnat
import time


def nat(n:int)->Node:


  lam0, x0 = lamvar()
  lam0.s0 = x0

  p = Node(Tag.Lam, lam0)

  for i in range(n):
    lam0 = Node(Tag.Lam)
    lams, xs = lamvar(lam0)
    lam0.s0 = xs(p)
    p = lams
  return p


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


def T()->Node: return Node(lambda t, f: t)

def F()->Node: return Node(lambda t, f: f)


def eq()->Node:
  return Y_comb()(
    lambda self, x, y: x(
      lambda px: y(
        lambda py: self(px, py),
        F
      ),
      y(
        lambda _: F,
        T
      )
    )
  )



def run_c_2_2():
  c = cnat(2)(cnat(2))
  print(c)
  for i in range(10):

    load_term_c(c)
    steps = run(20)
    if not steps: break
    c = unload_term_c()
    print(c)

  with hide_dups(True): print(c)



def run_scott_eq_3():
  N = 300
  c = eq()(nat(N), nat(N-1))
  
  load_term_c(c)
  st = time.time_ns()
  steps = run()
  dt = time.time_ns() - st
  print(f"{steps=} {dt/1e9} seconds {(steps/dt * 1e3):.3f} Mips")
  DEBUG.set(True)
  r = unload_term_c()
  print(r)
  with hide_dups(False): print(r)


def try_era_var():
  c = dup(Node(F))[0]
  print(c)
  c = run_term_c(c)
  print(c)

if __name__ == "__main__":  


  # try_era_var()

  # run_c_2_2()

  run_scott_eq_3()




  # st = time.time_ns()

  # B = 100000
  # for i in range(100):
  #   print(i)
  #   steps = run(B)
  #   if steps > -1:
  #     res = unload_term_c()
  #     t = time.time_ns() - st
  #     print(f"{t/1e9} seconds for {steps} steps, {steps/t*1e3:.3f} Mips")
  #     print(res)
  #     break
  pass
