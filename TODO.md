- Move the Scene Graph free detection to use empty slot free list instead of a "used" bit field.
  - https://ourmachinery.com/post/data-structures-part-1-bulk-data/
- Move window library to be linked statically instead of this registering nonsense.
  - Exactly as the renderer works.
- Add the inverse model matrix multiplication for normals in lighting.

