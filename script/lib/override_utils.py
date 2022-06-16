# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/. */

import contextlib
import inspect
import types
import os

_gn_args = None


def override_function(scope, name=None, condition=True):
    """Replaces an existing function in the scope."""

    def decorator(new_function):
        is_dict_scope = isinstance(scope, dict)
        function_name = name or new_function.__name__
        if is_dict_scope:
            original_function = scope.get(function_name, None)
        else:
            original_function = getattr(scope, function_name, None)

        if not callable(original_function):
            raise NameError(f'Failed to override function: '
                            f'{function_name} not found or not callable')

        def wrapped_function(*args, **kwargs):
            return new_function(original_function, *args, **kwargs)

        if not condition:
            wrapped_function = original_function

        if is_dict_scope:
            scope[function_name] = wrapped_function
        else:
            setattr(scope, function_name, wrapped_function)

        return wrapped_function

    return decorator


@contextlib.contextmanager
def override_scope_function(scope, new_function, name=None, condition=True):
    """Scoped function override helper. Can override a scope function or a class
    method."""
    if not condition:
        yield
        return

    function_name = name or new_function.__name__
    original_function = getattr(scope, function_name, None)
    try:
        if not callable(original_function):
            raise NameError(f'Failed to override scope function: '
                            f'{function_name} not found or not callable')

        if inspect.ismethod(original_function):

            def wrapped_method(self, *args, **kwargs):
                return new_function(self, original_function, *args, **kwargs)

            setattr(scope, function_name,
                    types.MethodType(wrapped_method, scope))
        else:

            def wrapped_function(*args, **kwargs):
                return new_function(original_function, *args, **kwargs)

            setattr(scope, function_name, wrapped_function)

        yield
    finally:
        if condition and original_function:
            setattr(scope, function_name, original_function)


@contextlib.contextmanager
def override_scope_variable(scope,
                            name,
                            value,
                            fail_if_not_found=True,
                            condition=True):
    """Scoped variable override helper."""
    if not condition:
        yield
        return

    var_exist = hasattr(scope, name)
    if fail_if_not_found and not var_exist:
        raise NameError(f'Failed to override scope variable: {name} not found')
    if var_exist:
        original_value = getattr(scope, name)
    try:
        setattr(scope, name, value)
        yield
    finally:
        if var_exist:
            setattr(scope, name, original_value)
        else:
            delattr(scope, name)


def get_gn_arg(arg, default_value=None, output_dir=None):
    """Returns GN arg from args.gn in output_dir."""
    global _gn_args
    if _gn_args is None:
        ARGS_GN = "args.gn"
        args_gn_file = None
        if output_dir is None:
            output_dir = os.getcwd()
        directory = output_dir
        while True:
            filepath = os.path.join(directory, ARGS_GN)
            if os.path.isfile(filepath):
                args_gn_file = filepath
                break
            dir_basename = os.path.basename(directory)
            if dir_basename in ["out", "src"]:
                # Stop at src/out or src dir.
                break
            parent_dir = os.path.dirname(directory)
            if parent_dir == directory:
                # We hit the system root directory.
                break
            directory = parent_dir

        if not args_gn_file:
            raise FileNotFoundError(f"{ARGS_GN} not found in: {output_dir}"
                                    f", search stopped at: {directory}")
        with open(args_gn_file, "r") as f:
            import gn_helpers  # pylint: disable=import-outside-toplevel
            _gn_args = gn_helpers.FromGNArgs(f.read())

    return _gn_args.get(arg, default_value)
