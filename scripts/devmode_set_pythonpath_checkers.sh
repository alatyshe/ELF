#!/usr/bin/env bash

export ELF_DEVELOPMENT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )"/.. && pwd )"
PYTHONPATH="${ELF_DEVELOPMENT_ROOT}"/src_py/:
PYTHONPATH+="${ELF_DEVELOPMENT_ROOT}"/build/elf/:
PYTHONPATH+="${ELF_DEVELOPMENT_ROOT}"/build/elfgames/checkers/:${PYTHONPATH}
export PYTHONPATH