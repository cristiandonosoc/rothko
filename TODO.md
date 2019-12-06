## Window

- Move window library to be linked statically instead of this registering nonsense.
  - Exactly as the renderer works.

## Scene

- Move the Scene Graph free detection to use empty slot free list instead of a "used" bit field.
  - https://ourmachinery.com/post/data-structures-part-1-bulk-data/

## Math

- Make a direct Quaternion->Euler extraction without the transform matrix roundtrip.

## glTF

- Detect repeated mesh and not reload them, correctly setting the new node.

## Imgui

- Unify new frame names to Update
- Change API to const ref.
- Simplify starting/usage.
