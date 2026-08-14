# Change Log
All notable changes to this project will be documented in this file as of Tiny v1.0

## Tiny 1.2.2

### Fixed

- Fixed top level module downloading

## Tiny 1.2.1

### Fixed

- Fixed module downloading into a nested copy
- Fixed module ignoring due to build folder

## Tiny 1.2.0

### Added

- Tiny now supports modules! Create a .tinymodule file to describe a module, and then another project can point to it via a git repository in order to download and package it!
- new PORT option, which creates a symlink into a working environment
- run flag to run in a working environment on build success
- clean flag to clean the build cache

## Tiny 1.1.14

### Added

- Tiny now supports multiple combined project directories, so if you define multiple they will be effectively merged during compilation

## Tiny 1.1.13

### Fixed

- Fixed audit issue where functions were no longer being parsed for non-static properties

## Tiny 1.1.12

### Fixed

- Audit no longer complains about static safety on comments

## Tiny 1.1.11

### Fixed

- Audit no longer complains about static safety on normal struct declarations

## Tiny 1.1.10

### Fixed

- Audit no longer complains about static safety for extern variables

## Tiny 1.1.9

### Fixed

- Global variables initialized with brackets or are non-initialized are now detected if non-static
- Reduced compile warnings

## Tiny 1.1.8

### Added

- new flag to recompile vendors
- can now comment out lines in .tinyconf with "#"

## Tiny 1.1.7

### Added

- audit now detects when variables and functions should be static

## Tiny 1.1.6

### Added

- added override option to pass in a raw argument to GCC
- added debug flag to check proceed final command buffer

## Tiny 1.1.5

### Added

- added mac support

## Tiny 1.1.4

### Changed

- audit will now first parse for an easyc header before parsing for unmonitored memory operations

### Fixed

- audit checks for nospaces between functions and braces now show the correct lines

## Tiny 1.1.3

### Added

- can now add compile definitions in .tinyconf via the DEFINE keyword

## Tiny 1.1.2

### Added

- command line args can now append =TRUE/FALSE to override true or false flags

## Tiny 1.1.1

### Added

- FLAG .tinyconf arg can now add arguments

## Tiny 1.1.0

### Fixed

- Fixed newlines being unhandled properly in .tinyconf

### Added

- Unknown arguments and flags will now error out with a message
- multithreaded flag for fast builds
- prod builds now do LTO (Link Time Optimization)

### Changed

- "CRITICAL FAILURE" changed to just "ERROR"
- now using crash handler for all errors

## Tiny 1.0.2

### Fixed

- Fixed typo saying vendors were compiled instead of sources

## Tiny [1.0.1]

### Added

- Main file can now be specified directly via path to file
