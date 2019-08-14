# Graphics

Rothko uses a renderer abstraction in order to be used by any graphics API that
can implements its API (should be most of them).

The interface in |rothko/graphics/renderer.h| is the one all backends must comply with.

Each implementation should be within a sub-directory.

