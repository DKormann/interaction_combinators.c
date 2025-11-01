from contextlib import contextmanager
import os


class Env:
  def __init__(self, name:str, value:int):
    self.name = name
    self.value = value
    if name in os.environ: self.value = int(os.environ[name])
    else: self.value = value
  
  def set(self, value:int): self.value = value
  def get(self)->int: return self.value
  def __str__(self)->str: return f"{self.value}"
  def __bool__(self)->bool: return bool(self.value)
  def __int__(self)->int: return int(self.value)
  def __lt__(self, other)->bool: return self.value < int(other)
  def __gt__(self, other)->bool: return self.value > int(other)
  def __eq__(self, other)->bool: return self.value == int(other)

  @contextmanager
  def __call__(self, value: int):
    old_value = self.value
    self.value = value
    try: yield
    finally: self.value = old_value


hide_dups = Env("hide_dups", False)
print_tree = Env("tree", True)
DEBUG = Env("DEBUG", 0)
TIMEIT = Env("TIMEIT", True)