# Change Log
All notable changes to this project will be documented in this file as of Tiny v1.0

## Tiny 1.1.1

### Added

- prod builds now do LTO (Link Time Optimization)

## Tiny 1.1.0

### Fixed

- Fixed newlines being unhandled properly in .tinyconf

### Added

- Unknown arguments and flags will now error out with a message
- multithreaded flag for fast builds

### Changed

- "CRITICAL FAILURE" changed to just "ERROR"
- now using crash handler for all errors

## Tiny 1.0.2

### Fixed

- Fixed typo saying vendors were compiled instead of sources

## Tiny [1.0.1]

### Added

- Main file can now be specified directly via path to file
