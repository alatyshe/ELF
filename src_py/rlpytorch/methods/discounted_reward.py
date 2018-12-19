# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import inspect
import os
from elf.options import auto_import_options, PyOptionSpec


class DiscountedReward(object):
    @classmethod
    def get_option_spec(cls):
        spec = PyOptionSpec()
        spec.addFloatOption(
            'discount',
            'exponential discount rate',
            0.99)
        return spec

    @auto_import_options
    def __init__(self, option_map):
        """Initialization of discounted reward."""
        pass

    def setR(self, R, stats):
        """Set rewards and feed to stats."""
        print("\x1b[1;33;40m|py|\x1b[0m", "DiscountedReward::", inspect.currentframe().f_code.co_name)
        # print("\t\x1b[1;33;40m", os.path.dirname(os.path.abspath(__file__)), " - ", os.path.basename(__file__), "\x1b[0m")

        self.R = R
        stats["init_reward"].feed(R.mean())

    def feed(self, batch, stats):
        """Update discounted reward and feed to stats.

        Keys in a batch:

        ``r`` (tensor): immediate reward.

        ``terminal`` (tensor): whether the current game has terminated.

        Feed to stats: immediate reward and accumulated reward
        """
        print("\x1b[1;33;40m|py|\x1b[0m", "DiscountedReward::", inspect.currentframe().f_code.co_name)
        # print("\t\x1b[1;33;40m", os.path.dirname(os.path.abspath(__file__)), " - ", os.path.basename(__file__), "\x1b[0m")
        
        r = batch["r"]
        term = batch["terminal"]

        # Compute the reward.
        self.R = self.R * self.options.discount + r

        # If we see any terminal signal, break the reward backpropagation
        # chain.
        for i, terminal in enumerate(term):
            if terminal:
                self.R[i] = r[i]

        stats["reward"].feed(r.mean())
        stats["acc_reward"].feed(self.R.mean())

        return self.R
