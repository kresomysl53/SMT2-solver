#!/usr/bin/env python3

import os

from tempfile import NamedTemporaryFile as TmpFile
from subprocess import run, PIPE, TimeoutExpired

from model import Model, Input, STATUS_SAT, STATUS_UNSAT, ModelError, InputError

TRANSLATOR = "../code/main"
SOLVER = "minisat"

RC_SAT = 10
RC_UNSAT = 20


class colors:
    red = "\033[91m"
    green = "\033[92m"
    white = "\033[m"


def print_ok(text):
    print(f"{colors.green}{text}{colors.white}")


def print_err(text):
    print(f"{colors.red}{text}{colors.white}")


class GeneratorError(Exception):
    pass


class SolverError(Exception):
    pass


def smoke_test():
    # Verify that MiniSat solver is available
    try:
        run([SOLVER, "--help"], stdout=PIPE, stderr=PIPE)
    except Exception:
        print("MiniSat is not installed or is not in PATH")
        exit(1)


def execute(path):
    with TmpFile(mode="w+") as dimacs_out, TmpFile(mode="w+") as model_out:
        try:
            translator = run([TRANSLATOR, path], stdout=dimacs_out, stderr=PIPE)
        except Exception:
            raise GeneratorError("Error when running formula generator")

        if translator.returncode != 0:
            raise GeneratorError(translator.stderr.decode().strip())

        try:
            solver = run(
                [SOLVER, dimacs_out.name, model_out.name], stdout=PIPE, stderr=PIPE
            )
        except Exception:
            raise SolverError("Error when running SAT solver")

        if not solver.returncode in [RC_SAT, RC_UNSAT]:
            raise SolverError(solver.stderr.decode().strip())

        input = Input.load(path)
        model = Model.load(model_out.name, input)
        return model


def run_test_case(path, expected_status):
    try:
        result = execute(path)
    except GeneratorError:
        print_err(f"{path}: Generator error")
        return
    except SolverError:
        print_err(f"{path}: SAT solver error")
        return

    if expected_status != result.status:
        print_err(
            f"{path}: Invalid result: got {result.status}, expected {expected_status}"
        )
    else:
       	try:
            if result.is_sat():
                result.check()
            print_ok(f"{path}: OK")
        except ModelError as e:
            print_err(f"{path}: {e}")


def run_test_suite(path, expected_status):
    for test_case in sorted(os.listdir(path)):
        if test_case.endswith(".in"):
            run_test_case(os.path.join(path, test_case), expected_status)


if __name__ == "__main__":
    smoke_test()
    run_test_suite("../tests/sat", STATUS_SAT)
    run_test_suite("../tests/unsat", STATUS_UNSAT)
