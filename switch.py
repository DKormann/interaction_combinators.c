#%%



from dataclasses import dataclass


class E:

  __match_args__ = ('a', 'b')
  def __init__(self, a: int, b: int = 2):
    self.a = a
    self.b = b



class F:
  __match_args__ = ('a', 'b')
  def __init__(self, a: int, b: int = 2):
    self.a = a
    self.b = b


e = E(1)


match e:
  case E(1,3): print("one")
  case E(2): print("two")
  case _: print("other")







