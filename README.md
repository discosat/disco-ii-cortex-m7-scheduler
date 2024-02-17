# Disco II Cortex-M7 scheduler
Source code and build setup for the Cortex M7 auxiliary core payload scheduler aboard the DISCO II student cubesat.

## TLDR
Got [docker](https://www.docker.com/)? Great, clone the repository (including submodules), open a shell and run:
```bash
docker compose build && docker compose up
```

This will compile and install a debug and release version into the `bin/` directory, ready to be loaded onto a PicoCore™MX8MP.
