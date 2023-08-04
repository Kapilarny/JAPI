
# Lua API Reference

## Global Variables

Every lua plugin has to define the following global variables:

### `config` (table)

#### `config.name` (string)

#### `config.guid` (string)

#### `config.version` (string)

#### `config.author` (string)

## Functions

### `log_trace(message)`

Logs a message with the `TRACE` level.

### `log_debug(message)`

Logs a message with the `DEBUG` level.

### `log_info(message)`

Logs a message with the `INFO` level.

### `log_warn(message)`

Logs a message with the `WARN` level.

### `log_error(message)`

Logs a message with the `ERROR` level.

### `log_fatal(message)`

Logs a message with the `FATAL` level.

### `patch_mem(address, data)`

Patches the memory at the given address with the given data.

### `patch_asbr_mem(offset, data)`

Patches the memory at the ASBR.exe moduleBase address + offset with the given data

### `copy_mem(address, size)`

Returns copied memory at the given address with the given size.

### `copy_asbr_mem(offset, size)`

Returns copied memory at the ASBR.exe moduleBase address + offset with the given size
