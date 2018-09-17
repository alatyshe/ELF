# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import inspect
import traceback
from collections import Counter


def move2xy(v):
    print("\x1b[1;32;40m" + "console_lib - move2xy() : " + "\x1b[0m")
    if v.lower() == "pass":
        return -1, -1
    x = ord(v[0].lower()) - ord('a')
    # Skip 'i'
    if x >= 9:
        x -= 1
    y = int(v[1:]) - 1
    return x, y


def xy2move(x, y):
    print("\x1b[1;32;40m" + "console_lib - xy2move() : " + "\x1b[0m")
    if x == -1 and y == -1:
        return "pass"

    if x >= 8:
        x += 1
    return chr(x + 65) + str(y + 1)















class GoConsoleGTP:
    def on_protocol_version(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_protocol_version() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\titems : ", items)
        print("\treply : ", reply)

        return True, "2"




    def on_clear_board(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_clear_board() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\titems : ", items)
        print("\x1b[1;33;40m", "\treply : ", reply, "\x1b[0m")

        reply["a"] = self.actions["clear"]
        return True, reply




    def on_name(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_name() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\titems : ", items)
        print("\treply : ", reply)

        return True, "DF2"




    def on_komi(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_komi() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\x1b[1;33;40m", "\titems : ", items, "\x1b[0m")
        print("\treply : ", reply)

        # For now we just fix komi number.
        if items[1] != "7.5":
            return False, "We only support 7.5 komi for now"
        return True, None




    def on_boardsize(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_boardsize() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\x1b[1;33;40m", "\titems : ", items, "\x1b[0m")
        print("\treply : ", reply)

        if items[1] != str(self.board_size):
            return (
                False,
                "We only support %dx%d board for now" % (
                    self.board_size, self.board_size)
            )
        return True, None




    def on_genmove(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_genmove() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\x1b[1;33;40m", "\titems : ", items, "\x1b[0m")
        print("\x1b[1;33;40m", "\treply : ", reply, "\x1b[0m")

        ret, msg = self.check_player(batch, items[1][0])
        if ret:
            reply["a"] = self.actions["skip"]
            return True, reply
        else:
            return False, msg




    def on_play(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_play() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\x1b[1;33;40m", "\titems : ", items, "\x1b[0m")
        print("\x1b[1;33;40m", "\treply : ", reply, "\x1b[0m")

        ret, msg = self.check_player(batch, items[1][0])
        if ret:
            reply["a"] = self.move2action(items[2])
            return True, reply
        else:
            return False, msg




    def on_showboard(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_showboard() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\titems : ", items)
        print("\treply : ", reply)

        self.showboard(batch)
        return True, None




    def on_final_score(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_final_score() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\titems : ", items)
        print("\treply : ", reply)

        final_score = self.get_final_score(batch)
        if final_score > 0:
            return True, "B+%.1f" % final_score
        else:
            return True, "W+%.1f" % (-final_score)




    def on_version(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_version() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\titems : ", items)
        print("\treply : ", reply)

        return True, "1.0"




    def on_exit(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_exit() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\titems : ", items)
        print("\x1b[1;33;40m", "\treply : ", reply, "\x1b[0m")

        self.exit = True
        return True, reply




    def on_quit(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_quit() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\x1b[1;33;40m", "\titems : ", items, "\x1b[0m")
        print("\x1b[1;33;40m", "\treply : ", reply, "\x1b[0m")

        return self.on_exit(batch, items, reply)




    def on_list_commands(self, batch, items, reply):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.on_list_commands() : " + "\x1b[0m")
        print("\tbatch : ", batch)
        print("\titems : ", items)
        print("\treply : ", reply)

        msg = "\n".join(self.commands.keys())
        return True, msg




    def __init__(self, GC, evaluator):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.__init__() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tGC : ", GC, "\x1b[0m")
        print("\x1b[1;33;40m", "\tevaluator : ", evaluator, "\x1b[0m")

        self.exit = False
        self.GC = GC
        self.board_size = GC.params["board_size"]
        self.evaluator = evaluator
        self.actions = {
            "skip": GC.params["ACTION_SKIP"],
            "pass": GC.params["ACTION_PASS"],
            "resign": GC.params["ACTION_RESIGN"],
            "clear": GC.params["ACTION_CLEAR"]
        }
        self.last_cmd = ""

        self.commands = {
            key[3:]: func
            for key, func in inspect.getmembers(
                self, predicate=inspect.ismethod)
            if key.startswith("on_")
        }




    def move2action(self, v):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.move2action() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tv : ", v, "\x1b[0m")

        if v.lower() in self.actions:
            return self.actions[v.lower()]

        x, y = move2xy(v)
        return x * self.board_size + y




    def actor(self, batch):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.actor() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")

        reply = self.evaluator.actor(batch)
        return reply




    def action2move(self, a):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.action2move() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\ta : ", a, "\x1b[0m")

        x = a // self.board_size
        y = a % self.board_size
        return xy2move(x, y)




    def showboard(self, batch):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.showboard() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")

        print(batch.GC.getGame(0).showBoard())




    def get_next_player(self, batch):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.get_next_player() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")

        return batch.GC.getGame(0).getNextPlayer()




    def get_last_move(self, batch):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.get_last_move() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")

        return batch.GC.getGame(0).getLastMove()




    def get_final_score(self, batch):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.get_final_score() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")

        return batch.GC.getGame(0).getLastScore()




    def check_player(self, batch, player):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.check_player() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\x1b[1;33;40m", "\tplayer : ", player, "\x1b[0m")

        board_next_player = self.get_next_player(batch)
        if player.lower() != board_next_player.lower():
            return (
                False,
                ("Specified next player %s is not the same as the "
                 "next player %s on the board") % (
                    player, board_next_player
                )
            )
        else:
            return True, None




    def print_msg(self, ret, msg):
        print("console_lib - GoConsoleGTP.print_msg() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tret : ", ret, "\x1b[0m")
        print("\x1b[1;33;40m", "\tmsg : ", msg, "\x1b[0m")

        print("\n%s %s\n\n" % (("=" if ret else "?"), msg))




    def prompt(self, prompt_str, batch):
        print("\x1b[1;32;40m" + "console_lib - GoConsoleGTP.prompt() : " + "\x1b[0m")
        print("\x1b[1;33;40m", "\tbatch : ", batch, "\x1b[0m")
        print("\x1b[1;33;40m", "\tprompt_str : ", prompt_str, "\x1b[0m")

        for i in self.commands:
            print(i, ", ", end='')
        print()
        # Show last command results.
        if self.last_cmd == "play" or self.last_cmd == "clear_board":
            self.print_msg(True, "")
        elif self.last_cmd == "genmove":
            self.print_msg(True, self.get_last_move(batch))

        self.last_cmd = ""

        while True:
            cmd = input(prompt_str)
            print("cmd : ", cmd)
            items = cmd.split()
            if len(items) < 1:
                self.print_msg(False, "Invalid input")
                continue

            c = items[0]
            reply = dict(pi=None, a=None, V=0)

            print("\x1b[1;32;40m" + "commands : " + "\x1b[0m")


            try:
                ret, msg = self.commands[c](batch, items, reply)
                self.last_cmd = c
                if not ret:
                    self.print_msg(False, msg)
                else:
                    if isinstance(msg, dict):
                        return msg
                    elif isinstance(msg, str):
                        self.print_msg(True, msg)
                    else:
                        self.print_msg(True, "")

            except Exception:
                print(traceback.format_exc())
                self.print_msg(False, "Invalid command")
