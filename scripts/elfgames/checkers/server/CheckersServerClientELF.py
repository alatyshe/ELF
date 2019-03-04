import sys

import random
import numpy as np
import socket
import json

from time import sleep

class RemoteCheckersPlayerWebSocketsELF():
    def __init__(self, game, connection, sid,
                 debugNnetAgent=None,
                 debugMCTSAgent=None,
                 send_mcts_stats_enabled=False):

        super().__init__(name="Human via Web Socket Client")

        self.game = game
        self.connection = connection
        self.sid = sid
        self.debugNnetAgent = debugNnetAgent
        self.debugMCTSAgent = debugMCTSAgent
        self.send_mcts_stats_enabled = send_mcts_stats_enabled

        self.victory = None
        self.RESET_GAME_CODE = -1
        self.INTERRUPT_GAME_CODE = -2
        self.interrupted = False
        self.message = None
        self.last_player = None

        print("Connection was established")

    def prepare_to_game(self):
        pass

    def send_state(self, game, game_player):
        print("sending game info...")

        canonical_state = game.get_observation(game_player)

        valid = game.get_valid_moves(game_player)
        all_moves = game.board_impl.get_all_moves()

        move_pairs_dict = {}

        for idx in range(len(valid)):
            if valid[idx]:
                move_pairs_dict[idx] = all_moves[idx]
                print("", idx, " => ", all_moves[idx])

        data = {
            'type': 'game_state',
            'data': {
                'state': canonical_state.tolist(),
                'moves': move_pairs_dict
            },
            'sid': self.sid
        }

        json_data = json.dumps(data)

        self.send_message(json_data.encode("utf-8"))

        print("game info sent!")

    def send_action_estimator_data(self, game, game_player, agent, type):
        print("sending game info...")

        if game_player == (game.get_players_num() - 1):
            opponent_player = 0
        else:
            opponent_player = game_player + 1

        if game.get_cur_player() != opponent_player:
            return

        history_n = game.history_n
        game.history_n = 0
        canonical_state = game.get_observation(opponent_player)
        game.history_n = history_n

        actions, value = agent.predict(game, opponent_player)

        actions = list(np.around(np.array(actions), 2))

        valid = game.get_valid_moves(opponent_player)
        all_moves = game.board_impl.get_all_moves()

        move_pairs_dict = {}
        prob_pairs_dict = {}

        for idx in range(len(valid)):
            if valid[idx] > 0:
                move_pairs_dict[idx] = all_moves[idx]
                if actions[idx] > 0:
                    prob_pairs_dict[idx] = float(actions[idx])

        data = {
            'type': type,
            'data': {
                'state': canonical_state.tolist(),
                'moves': move_pairs_dict,
                'action_probabilities': prob_pairs_dict
            },
            'sid': self.sid
        }

        json_data = json.dumps(data)

        self.send_message(json_data.encode("utf-8"))

        print("action estimator data sent!")

    def send_action_q_value_estimator_data(self, game, game_player):
        print("sending game info...")

        if game_player == (game.get_players_num() - 1):
            opponent_player = 0
        else:
            opponent_player = game_player + 1

        history_n = game.history_n
        game.history_n = 0
        canonical_state = game.get_observation(opponent_player)
        game.history_n = history_n

        all_moves = game.board_impl.get_all_moves()
        valid = game.get_valid_moves(opponent_player)

        move_pairs_dict = {}
        q_pairs_dict = {}

        for idx in range(len(valid)):
            if valid[idx] > 0:
                game_clone = game.clone()
                game_clone.make_move(idx)
                _, value = self.debugAgent.predict(game_clone, opponent_player)

                move_pairs_dict[idx] = all_moves[idx]
                q_pairs_dict[idx] = value

        data = {
            'type': 'action_q_value_estimator_data',
            'data': {
                'state': canonical_state.tolist(),
                'moves': move_pairs_dict,
                'moves_q_values': q_pairs_dict
            },
            'sid': self.sid
        }

        json_data = json.dumps(data)

        self.send_message(json_data.encode("utf-8"))

        print("action estimator data sent!")

    def send_error(self, message):
        data = {'type': 'error', 'data': message, 'sid': self.sid}

        json_data = json.dumps(data)

        self.send_message(json_data.encode("utf-8"))

    def send_message(self, message):
        print("send_message ", message)

        self.connection.sendMessage(message)

    def interrupt(self):
        self.interrupted = True

    def on_message(self, data):
        self.message = data

    def on_turn_finished(self, game):
        if self.send_mcts_stats_enabled:
            if self.debugNnetAgent is not None and not game.is_ended():
                self.send_action_estimator_data(game, self.last_player, self.debugNnetAgent,
                                                'nnet_action_estimator_data')
            if self.debugMCTSAgent is not None and not game.is_ended():
                self.send_action_estimator_data(game, self.last_player, self.debugMCTSAgent,
                                                'mcts_action_estimator_data')

    def predict(self, game, game_player):
        self.last_player = game_player

        valid_moves = game.get_valid_moves(game_player)

        action_idx = None

        print("waiting for a move...")

        self.send_state(game, game_player)

        while self.victory is None:
            if self.interrupted:
                print("game interrupted!")
                return self.INTERRUPT_GAME_CODE

            if self.message is None:
                sleep(0.2)
                continue

            try:
                print("received message: \n", self.message)
                json_message = json.loads(self.message)
            except:
                print("Could not parse json message")
                self.send_error("Could not parse json message")
                self.message = None
                continue

            if json_message["type"] == "get_state":

                self.send_state(game, game_player)

            elif json_message["type"] == "action":

                data = json_message["data"]

                try:
                    action_idx = int(data)

                    if action_idx == self.RESET_GAME_CODE:
                        break
                    elif action_idx == self.INTERRUPT_GAME_CODE:
                        self.interrupted = True
                        break
                    elif valid_moves[action_idx]:
                        break
                    else:
                        self.send_error('Invalid move index')

                except ValueError:
                    self.send_error("Could not parse move index")
            else:
                self.send_error("Could not parse move index")

            self.message = None

        self.message = None

        print("received action ", action_idx)

        actions = [0] * len(valid_moves)
        actions[action_idx] = 1

        return actions, -1

    def on_game_ended(self, won):
        self.victory = won

        if not self.interrupted:
            data = {'type': 'end_game', 'sid': self.sid}
            if self.victory:
                data['data'] = 1
            else:
                data['data'] = 0


            json_data = json.dumps(data)

            self.send_message(json_data.encode("utf-8"))

        print("Game ended. Player won: ", self.victory)
