#!/usr/bin/env python3

import sys

from model import ModelError
from run_tests import execute, smoke_test, print_ok, print_err, SolverError, GeneratorError


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: ./run.py input")
        exit(1)

    smoke_test()

    try:
        result = execute(sys.argv[1])
    except GeneratorError as e:
        print_err("Generator error:")
        print(e)
        exit(1)
    except SolverError as e:
        print_err("SAT solver error:")
        print(e)
        exit(1)

    result.print()

    if result.is_sat():
        try:
            result.check()
            print_ok("Found model is correct")
        except ModelError as e:
            print_err(f"\n{e}")
            exit(1)
