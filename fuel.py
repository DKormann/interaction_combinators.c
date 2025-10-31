


def fuel(arg):
  try:
    ev = eval(arg)
    return str(int(ev*100)) + "%"

  except Exception as e: print("error:", e)
  return ""


def test(inp, out):
  res = fuel(inp)
  if res != out:
    print(f"Test failed for input {inp} and output {res} expected {out}")


test("0", "0%")
test("1", "100%")
test("1/2", "50%")
test("1/4", "25%")
test("2/3", "66%")




def main():
  var = input("Enter a number: ")
  print(fuel(var))


