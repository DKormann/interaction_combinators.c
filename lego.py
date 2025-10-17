


#%%


canvas = []


def block(x:int, y:int, c:chr):
  if x < 0 or y < 0: return
  while len(canvas) <= y:
    canvas.append([])
  while len(canvas[y]) <= x:
    canvas[y].append(" ")
  canvas[y][x] = c


def floor(x:int, y:int, width:int):
  for i in range(width):
    block(x + i, y, "-")

def wand(x:int, y:int, height:int):
  for i in range(height):
    block(x, y + i, "#")

def dach(x:int, y, width:int):
  for i in range(width):
    block(x + i, y - i, "\\")
    block(x - i, y - i, "/")
  block(x, y, "*")


def haus(x:int, height:int, width:int):
  wand(x + 5,0,height)
  wand(x + 5 + width,0,height)
  dach(x + 5 + width//2,height + width//2+1,width//2+3)
  floor(x + 5,height, width +1)

def tanne(x:int, height:int):
  wand(x, 0, height)
  wand(x+2,0, height)
  for i in range(height//3, height):
    dach(x + 1, i *2, height - i)


haus(1,6,10)
haus(16,8,8)
haus(30,5,16)
floor(0,0,60)

tanne(60,12)

print("\n".join("".join(row) for row in reversed(canvas)))