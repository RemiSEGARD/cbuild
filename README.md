# CBuild

Header library to write a bootstrapable build system in C only.
Inspired by Tsoding's [nob](https://github.com/tsoding/nobuild).

Currently only compatible with Unix systems.

# How to use

See [examples](./examples/)

```
cc -o cbuild cbuild.c
```

# Arguments

You can use [cargparse](https://github.com/RemiSEGARD/cargparse) to read arguments
from the CLI. Simply copy the `cargparse.h` header next to the `cbuild` source.

In order to add your own arguments, you can use the `CBUILD_CUSTOM_ARGS` macro.

For more information, have a look at [with_cargparse](./examples/with_cargparse/)
