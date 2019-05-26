# Window management

Rothko uses a window abstraction in order to be used by any window manager
flexible enough to conform to its API, which should most of them.

The rest of the code should only use whatever is defined in `common`, which is
the code meant to only deal with Rothko's abstraction.

The rest of the code is implementation details of each window manager backend.
See `common/window.h` and `common/window_backend.h` for more details.
