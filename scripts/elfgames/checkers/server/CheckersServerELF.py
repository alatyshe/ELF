import os
import sys
import copy

# comment it
# sys.path.insert(0, '../../..')

# from core.World import World
# from core.utils import *
# from EnvironmentSelector import EnvironmentSelector
# comment end



import numpy as np
import glob as glob
import os.path
import collections
import socket
import json
import sys
import uuid

from time import sleep
from _thread import start_new_thread
from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket
from CheckersServerClient import RemoteCheckersPlayerWebSockets

import pdb

from rlpytorch import Evaluator, load_env


from WrapperPlayerELF import WrapperPlayerELF

CATCH_CLIENT_EXCEPTIONS = True
DEBUG_ENABLED = True
FLIP_SIDES = True

game_storage = {}

additional_to_load = {
    'evaluator': (
        Evaluator.get_option_spec(),
        lambda object_map: Evaluator(object_map, stats=None)),
}

env = load_env(
    os.environ,
    overrides={
        'num_games': 1,
        'greedy': True,
        'T': 1,
        'model': 'online',
        'additional_labels': ['checkers_aug_code', 'checkers_move_idx'],
    },
    additional_to_load=additional_to_load)


print("haha")
evaluator = env['evaluator']

GC = env["game"].initialize()

model_loader = env["model_loaders"][0]
model = model_loader.load_model(GC.params)

mi = env['mi']
mi.add_model("model", model)
mi.add_model("actor", model)
mi["model"].eval()
mi["actor"].eval()

LOOP_END = False

class Server():
    def __init__(self, port):
        self.host = ''  # all available interfaces
        self.port = port


    def start(self):
        try:
            server = SimpleWebSocketServer('', self.port, WebSocketServerClient)

            print("listening for incoming connections...")
            server.serveforever()
        except:
            print("Could not create socket.")
            sys.exit(0)


class WebSocketServerClient(WebSocket):
    player = None
    connection = None

    def handleMessage(self):
        print("handle message: ", self.data)

        print(self.player)

        if self.player is not None:
            self.player.on_message(self.data)
            return

        if self.connection:
            json_message = json.loads(self.data)

            debug = False
            if "debug" in json_message:
                debug = json_message["debug"]

            if json_message["type"] == "get_state":
                if "sid" in json_message:
                    sid = json_message["sid"]
                    if sid in game_storage:
                        print("load game from game storage")
                        start_new_thread(self.build_game, (game_storage[sid], sid, debug,))
                        return

                print("build new game")

                sid = self.generate_new_sid()
                game_storage[sid] = GC

                start_new_thread(self.build_game, (GC, sid, debug,))

    def generate_new_sid(self):
        return "sid_" + uuid.uuid4().hex[:8]

    def handleConnected(self):
        print(self.address, ' connected')
        self.connection = self

    def handleClose(self):
        if self.player is not None:
            self.player.interrupt()

    def build_game(self, GC, sid, debug):
        print("build game")

        self.player = WrapperPlayerELF(GC, evaluator, self.connection, sid,
                                debugNnetAgent=None,
                                debugMCTSAgent=None)


        self.start_game(None, GC)

    def start_game(self, agents, GC):

        loop_end = False
        # console = GoConsoleGTP(GC, evaluator)

        console = self.player

        def human_actor(batch):
            return console.prompt("", batch)


        def actor(batch):
            return console.actor(batch)


        def train(batch):
            console.prompt("DF Train> ", batch)

        # def game_end(batch):
        #     nonlocal loop_end
        #     loop_end = True

        evaluator.setup(sampler=env["sampler"], mi=mi)

        GC.reg_callback_if_exists("checkers_actor_black", actor)
        GC.reg_callback_if_exists("human_actor", human_actor)
        GC.reg_callback_if_exists("train", train)
        # GC.reg_callback_if_exists("game_end", game_end)

        GC.start()
        GC.GC.getClient().setRequest(
            mi["actor"].step, -1, -1)

        evaluator.episode_start(0)

        while not loop_end:
            GC.run()
            if console.exit:
                break
        GC.stop()



if __name__ == "__main__":
    port = 8888

    # if len(sys.argv) > 1 and sys.argv[1] is not None:
    #     port = int(sys.argv[1])

    print("port: ", port)

    server = Server(port)

    server.start()
