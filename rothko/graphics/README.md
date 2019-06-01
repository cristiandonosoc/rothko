# Graphics

Rothko uses a renderer abstraction in order to be used by any graphics API that
can implements its API (should be most of them).

The rest of the code should only use elements defined in `rothko/common`.

Each implementation should be within a sub-directory and be sure to suscribe
the factory functions.

See `rothko/common/renderer.h` and `rothko/common/renderer_backend.h` for more
details.
