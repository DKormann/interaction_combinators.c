
from typing import Callable
from node import Node, Tag, app, hide_dups, lam, move, null, parse_lam, print_tree, sup, x, dup

def id(): return lam(x(0))

def T()->Node: return lam(lam(x(1)))
def F()->Node: return lam(lam(x(0)))

def cnat(n:int):
  def go(n:int):
    if n == 0: return x(0)
    return app(x(1), go(n-1))
  return lam(lam(go(n)))

def circular(option1:int, option2:int):
  aux = [null(), null()]
  dups = dup(sup(aux[0], aux[1], 0), 0)
  move(dups[option1], aux[option2])
  return dups[not option1]

def snat(n:int):
  if n == 0: return lam(lam(x(0)))
  return lam(lam(app(x(1), snat(n-1))))

def appn(a:Node, *arms):
  if not arms: return a
  appn(app(a, arms[0]), *arms[1:])


def fmt_eq(a:Node, b:Node):
  return str(a) == str(b)

if __name__ == "__main__":
  from main import run_term_c
  a = Node(lambda x, y: y)
  a = a(1)
  hide_dups.set(True)
  print(a)
  hide_dups.set(False)
  print(a)


  # print(circular(0, 1))
