# Carmen

## Base

### purpose / scope

A really small language that is easy to learn, with little
tradeoffs/abstractions.

### specs

- **paradigm**: procedural
- **typing**: static and strong
- **memory model**: user handles the memory

## features

base features:
- variables
- structs
- enums
- unions
- functions/procedures
- branching and looping

posible features:
- homonyms (function overloading)
- templates
- type inference
- comp time
- lambdas

## syntax base

```
# generic schema
[SCOPE] IDENT [: TYPE] [= VAL];

# vars
foo : int;  # declare
foo = 3;    # asign
bar : const int = 4; # declare + asign

# functions and procedures
- foo : func(int, int) -> (int) = (a, b) -> { ret a + b; };
+ foo : proc(float, float) -> (float) = (a, b) -> { ret a * b; };

# enum
+ NUMS : enum(char) {
    ONE, TWO, TREE,
};

# defines a struct to be used
- Vec2 : struct() {
    x: int;
    y: int;
};
```

