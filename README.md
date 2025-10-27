

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

## Thread Safety & Concurrent Testing

The C runtime has been refactored to support concurrent testing via pytest-xdist and similar parallel test frameworks. Key changes:

### C Code (`main.c`)
- **Removed global `runtime` variable**: All functions now accept `Runtime* runtime` as a parameter
- **New functions**:
  - `Runtime* new_runtime()`: Creates a new isolated runtime instance
  - `void free_runtime(Runtime* runtime)`: Cleans up a runtime instance
- **Updated function signatures**: All functions that use runtime now take it as a parameter:
  - `new_node(Runtime* runtime, Tag tag, int label)`
  - `free_node(Runtime* runtime, Node* node)`
  - `dup()`, `sup()`, `app()` - all take runtime parameter
  - `erase()`, `move()`, `just_move()` - pass runtime through
  - `step()`, `run()`, `load()`, `unload()` - accept runtime instance

### Python Interface (`main.py`)
- **Thread-local storage**: Uses `threading.local()` to store per-thread C library instances
- **`get_lib()` function**: Returns thread-specific CDLL instance
- **Updated C bindings**:
  - `lib.new_runtime()`: Creates runtime instance
  - `lib.free_runtime(runtime)`: Destroys runtime instance
  - All C functions updated to accept runtime pointer parameter
- **Thread-safe workflow**:
  1. Each thread gets its own C library instance via `get_lib()`
  2. Each test creates its own isolated runtime with `new_runtime()`
  3. Tests can run concurrently without shared state conflicts

### Usage
Tests can now run in parallel without conflicts:
```bash
pytest tests.py -n auto  # Run with pytest-xdist
```

## TODO:

 - [x] python runtime
 - [x] parse python ast to C and convert back from C
 - [ ] C runtime:
    - [x] basic reduction (lazy)
    - [x] circular DUP_SUP
    - [x] full reduction (find all possible terms in final form)
    - [ ] garbage collection
      - [ ] correctly collect dup targets
    - [ ] parallel



## citations

[blog by enrico borba](https://ezb.io/thoughts/interaction_nets/lambda_calculus/2025-08-30_lazy-memory-layout.html)

[gists by victor taelin](gist.github.com/VictorTaelin)
