# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os 
import inspect
from elf.options import auto_import_options, PyOptionSpec


class MoreLabels(object):
    @classmethod
    def get_option_spec(cls):
        spec = PyOptionSpec()
        spec.addStrListOption(
            'additional_labels',
            'add additional labels in the batch; e.g. id, seq, last_terminal',
            [])
        return spec

    @auto_import_options
    def __init__(self, option_map):
        pass

    def add_labels(self, desc):
        # print("\x1b[1;33;40m|py|\x1b[0m", "MoreLabels::", inspect.currentframe().f_code.co_name)
        # print("\t\x1b[1;33;40m", os.path.dirname(os.path.abspath(__file__)), " - ", os.path.basename(__file__), "\x1b[0m")

        if self.options.additional_labels:
            for _, v in desc.items():
                v["input"].extend(self.options.additional_labels)
