import unittest

from example import circular, cnat

# from functions import scott
import scott  
from main import run_term_c
from node import DEBUG, Node, hide_dups, lam, null, print_tree, reset_labels, x


def assert_fmt(term:Node, expected:str):
  with print_tree.context(False):
    assert str(term) == expected, f"Expected {expected}, got {str(term)}"

def assert_normal_fmt(term:Node, expected:str):
  term = run_term_c(term)
  assert_fmt(term, expected)    


class TestFormat(unittest.TestCase):
  def test_fmt(self):
    term = Node(lambda x: x)
    assert_fmt(term, "λa a")
    assert_fmt(term(1), "( λa a 1)")
  
  def test_fmt_app(self):
    term = Node(lambda x: x)
    assert_fmt(term, "λa a")
    assert_fmt(term(1), "( λa a 1)")
  
  def test_fmt_cnat(self):

    with hide_dups.context(False):
      assert_fmt(cnat(1), "λa λb ( a b)")
      assert_fmt(cnat(2), "λa λb ( &71{c, d} = a in c ( d b))")
  
  def test_fmt_snat(self):
    assert_fmt(scott.nat(0), "λ λa a")
    assert_fmt(scott.nat(1), "λa λ ( a λ λb b)")


class TestNormalization(unittest.TestCase):

  def test_id(self):
    def iden(): return Node(lambda x: x)
    assert_normal_fmt(iden(), "λa a")
    assert_normal_fmt(iden()(null()), "Nul")
  
  def test_circular(self):
    for i in range(2):
      for j in range(2):
        term = circular(i, j)
        assert_normal_fmt(term, "Nul")
  





