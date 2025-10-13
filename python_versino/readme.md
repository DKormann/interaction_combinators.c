

## python IC

hierarchical layout
generally usages point to values
so positive ports point to negative.

eg:

app points to fun and arg
sup points to both values
dup:
  dup2 points to dup
  dup points to dup2 and target
lam:
  points to body and var(!)
var: points to lam

var <-> lam connection:
  var and lam point to each other and are pointed to from outside, meaning every time you move lam or var you need to update outside references


