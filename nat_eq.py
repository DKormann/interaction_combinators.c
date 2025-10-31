
#%%


import sys
import time


sys.setrecursionlimit(10000)

def nat(n):
  if n == 0 : return lambda s,z:z

  for i in range(n):
    term = lambda s,z: s(nat(n-1))
  return term


T = lambda t,f:t
F = lambda t,f:f

def eq(a,b):
  return a(
    lambda p:b(
      lambda q: eq(p,q),
      F
    ),
    b(
      lambda q: F,
      T
    )
  )

def toint(c):
  return c(
    lambda p: 1 + toint(p),
    0
  )




N = 300


a = nat(N)
b = nat(N)

st = time.time_ns()
eq(a, b)(1,0)

dt = time.time_ns() - st

print(f"{dt/1e9} seconds")
