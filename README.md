

## IC runtime

its suprisingly easy to implement a full interaction based runtime in python and in C:

the approach here is to view interaction calculus more as an AST as they would appear in a standart language not as the interaction combinator graph itself.
this means terms point to their subterms. This is isomorphic to a subset of possible IC graphs namely those IC graphs corresponding to interaction calculus terms.

hierarchical layout

generally usages point to values
so positive ports point to negative.

eg:

app points to fun and arg
sup points to both values
dup / dup2:
  point to target and each other
lam:
  points to body and var
var: points to lam

var <-> lam connection:
  var and lam point to each other and are pointed to from outside, meaning every time you move lam or var you need to update outside references


implementing python runtime is quite 

## TODO:

 - [x] python runtime
 - [x] parse python ast to C and convert back from C
 - [ ] C runtime:
    - [x] basic reduction (lazy)
    - [ ] full reduction (find all possible terms in final form)
    - [ ] garbage collection
    - [ ] parallel



## citations

[blog by enrico borba](https://ezb.io/thoughts/interaction_nets/lambda_calculus/2025-08-30_lazy-memory-layout.html)

[gists by victor taelin](gist.github.com/VictorTaelin)
