# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Changed
- Changed auto-detection behaviour of serial port in `SerialReader`.  The old
  behaviour was to always first attempt to auto-detect and only consider the
  port provided by the user if this fails.  The new behaviour depends on the
  value of the `serial_port` argument:

  - Empty or "auto": Try to auto-detect the port.
  - any other value: Try to open the specified port.  Do not attempt to
    auto-detect.


## [1.0.0] - 2022-11-16
Initial version.


[Unreleased]: https://github.com/open-dynamic-robot-initiative/slider_box/compare/1.0.0...HEAD
[1.0.0]: https://github.com/open-dynamic-robot-initiative/slider_box/releases/tag/v1.0.0
