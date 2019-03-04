# import sys

# sys.path.insert(0, '../../..')

# from core.World import World
# from core.utils import *
# from EnvironmentSelector import EnvironmentSelector

# import numpy as np
# import glob as glob
# import os.path
# import collections
# import socket
# import json
# import sys
# import uuid

# from time import sleep
# from _thread import start_new_thread
# from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket
# from CheckersServerClient import RemoteCheckersPlayerWebSockets

# CATCH_CLIENT_EXCEPTIONS = True
# DEBUG_ENABLED = True
# FLIP_SIDES = True

# world = World()
# game_storage = {}

# # checkpoint = "models/best.h5"
# checkpoint = "models/save-609280.bin" # path to model

# # agent_profile_key = "checkers_agent_test_agent_rcnn_default"
# agent_profile_key = "checkers_elf_agent"

# env_selector = EnvironmentSelector()
# agent = env_selector.get_agent(agent_profile_key)
# agent.set_exploration_enabled(False)
# agent.load(checkpoint)

# agent_profile = env_selector.get_profile(agent_profile_key)
# game = env_selector.get_game(agent_profile.game)

# agent.disable_training_capability(temp_dir="temp", optimize=True)

# print("loaded checkpoint: ", checkpoint)


# class Server():
#     def __init__(self, port):
#         self.host = ''  # all available interfaces
#         self.port = port

#     def start(self):
#         try:
#             server = SimpleWebSocketServer('', self.port, WebSocketServerClient)

#             print("listening for incoming connections...")
#             server.serveforever()
#         except:
#             print("Could not create socket.")
#             sys.exit(0)


# class WebSocketServerClient(WebSocket):
#     player = None
#     connection = None

#     def handleMessage(self):
#         print("handle message: ", self.data)

#         if self.player is not None:
#             self.player.on_message(self.data)
#             return

#         if self.connection:
#             json_message = json.loads(self.data)

#             debug = False
#             if "debug" in json_message:
#                 debug = json_message["debug"]

#             if json_message["type"] == "get_state":
#                 if "sid" in json_message:
#                     sid = json_message["sid"]
#                     if sid in game_storage:
#                         print("load game from game storage")
#                         start_new_thread(self.build_game, (game_storage[sid], sid, debug,))
#                         return

#                 print("build new game")

#                 sid = self.generate_new_sid()
#                 client_game = game.clone()
#                 game_storage[sid] = client_game

#                 start_new_thread(self.build_game, (client_game, sid, debug,))

#     def generate_new_sid(self):
#         return "sid_" + uuid.uuid4().hex[:8]

#     def handleConnected(self):
#         print(self.address, ' connected')
#         self.connection = self

#     def handleClose(self):
#         if self.player is not None:
#             self.player.interrupt()

#     def build_game(self, game, sid, debug):
#         print("build game")

#         agent_clone = agent.clone()

#         if debug or DEBUG_ENABLED:
#             print("send debug info")
#             self.player = RemoteCheckersPlayerWebSockets(game, self.connection, sid,
#                                                          debugNnetAgent=None,
#                                                          debugMCTSAgent=agent_clone,
#                                                          send_mcts_stats_enabled=True)
#         else:
#             self.player = RemoteCheckersPlayerWebSockets(game, self.connection, sid,
#                                                          debugNnetAgent=None,
#                                                          debugMCTSAgent=None)

#         if not FLIP_SIDES:
#             agents = [self.player, agent]
#         else:
#             agents = [agent, self.player]

#         if CATCH_CLIENT_EXCEPTIONS:
#             try:
#                 self.start_game(agents, game)
#             except:
#                 print("Exception while playing the game!")
#                 self.player.interrupt()
#         else:
#             self.start_game(agents, game)

#     def start_game(self, agents, game):
#         game_experience, rewards = world.execute_game(agents, game,
#                                                       allow_exploration=False,
#                                                       verbose=True,
#                                                       show_every_turn=True,
#                                                       need_reset=False)

#         sess_1_first_won = rewards[0]
#         sess_1_second_won = rewards[1]

#         if not FLIP_SIDES:
#             self.player.on_game_ended(sess_1_first_won > sess_1_second_won)
#         else:
#             self.player.on_game_ended(sess_1_second_won > sess_1_first_won)


# if __name__ == "__main__":
#     port = 8888

#     if len(sys.argv) > 1 and sys.argv[1] is not None:
#         port = int(sys.argv[1])

#     print("port: ", port)

#     server = Server(port)

#     server.start()
