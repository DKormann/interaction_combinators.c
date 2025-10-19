import unittest

from example import circular, cnat
from main import run_term_c
from node import Node, hide_dups, lam, print_tree, reset_labels, x




def assert_fmt(term:Node, expected:str):
  with print_tree.context(False):
    assert str(term) == expected, f"Expected {expected}, got {str(term)}"

def assert_normal_fmt(term:Node, expected:str):
  term = run_term_c(term)
  assert_fmt(term, expected)
  hide_dups.set(True)
  assert_fmt(
    cnat(2),
    "λa λb app a app a b"
  )
  hide_dups.set(False)
  reset_labels()
  assert_fmt(
    cnat(2),
    "λa λb app 71{c, d} = a in c app d b"
  )
    


class TestFormat(unittest.TestCase):
  def test_fmt(self):
    term = lam(x(0))
    assert_fmt(term, "λa a")

class TestCircular(unittest.TestCase):
  
  def test_circular(self):
    for i in range(2):
      for j in range(2):
        term = circular(i, j)
        assert_normal_fmt(term, "Nul")

