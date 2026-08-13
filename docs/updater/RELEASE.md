# doc that documents how JAPI updates work

Update packages are packed into a zip, which is also signed

## important files

- manifest.json - contains metadata on the update, the update version and file meta
- dependencies.json - contains metadata on dlls required within this update, lib_load_order.txt is generated from this
