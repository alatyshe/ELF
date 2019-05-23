#!/usr/bin/env python

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os
import sys
import random
import torch

from time import sleep
from _thread import start_new_thread
from CheckersGui import boardToJsonCheckers
from rlpytorch import Evaluator, load_env
from py.CheckersMoves import get_all_moves_russian_checkers

from flask import Flask, session, redirect, url_for, request, render_template
import json

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
    'additional_labels': ['checkers_aug_code', 'checkers_move_idx'],
  },
  additional_to_load=additional_to_load)




all_session = {}


moves_for_human = get_all_moves_russian_checkers()


def init_observation(player_id):
  global env
  evaluator = env['evaluator']
  GC = env["game"].initialize()
  model = env["model_loaders"][0].load_model(GC.params)

  mi = env['mi']
  # mi.add_model("actor", model)

  if "actor" not in mi:
    mi.add_model("actor", model)
  else:
    mi.update_model("actor", model)

  mi["actor"].eval()

  user_id = player_id
  # Describe more!
  def human_actor(batch):
    global all_session

    all_session[user_id]["current_board"] = batch.GC.getGame(0).getBoard()
    all_session[user_id]["current_player"] = batch.GC.getGame(0).getCurrentPlayer()
    all_session[user_id]["current_valid_moves"] = {}

    valid_moves_binary = batch.GC.getGame(0).getValidMoves()

    for idx in range(len(valid_moves_binary)):
      if valid_moves_binary[idx]:
        i = "{0:036b}".format(moves_for_human[idx][0])[::-1]
        index = [pos for pos, char in enumerate(i) if char == "1"]
        i1, i2 = index
        buff1 = (1 + i1 - i1 // 9) - 1
        buff2 = (1 + i2 - i2 // 9) - 1
        x1, y1 = (6 - (buff1) % 4 * 2 + ((buff1) // 4) % 2, 7 - (buff1) // 4)
        x2, y2 = (6 - (buff2) % 4 * 2 + ((buff2) // 4) % 2, 7 - (buff2) // 4)
        if not moves_for_human[idx][1]:
          x1, y1, x2, y2 = x2, y2, x1, y1

        print("", idx, "\t: ", (y1 * 8 + x1), "=>", (y2 * 8 + x2))

        str_move_from = "y_" + str(y1) + " x_" + str(x1)
        str_move_to = "y_" + str(y2) + " x_" + str(x2)

        all_session[user_id]["current_valid_moves"][idx] = [str_move_from, str_move_to]

    all_session[user_id]["status_updated"] = True

    all_session[user_id]["reply"]["a"] = None
    all_session[user_id]["reply"]["pi"] = None
    all_session[user_id]["reply"]["checkers_V"] = None

    while not all_session[user_id]["response_filled"]:
      sleep(0.2)
    all_session[user_id]["response_filled"] = False

    return all_session[user_id]["reply"]

  def actor_black(batch):
    return evaluator.actor(batch)

  evaluator.setup(sampler=env["sampler"], mi=mi)


  GC.reg_callback_if_exists("human_actor", human_actor)
  GC.reg_callback_if_exists("checkers_actor_black", actor_black)
  GC.start()
  GC.GC.getClient().setRequest(
    mi["actor"].step, -1, -1)

  evaluator.episode_start(0)

  while True:
    GC.run()

    if all_session[user_id]["game_exit"]:
      break
  # fix this for normal exit
  # sys.exit()
  GC.stop()



















app = Flask(__name__)
app.secret_key = 'SECRET_KEY'

@app.route('/', methods=['GET', 'POST'])
def main():
  # проверяем на наличие сессии
  user_id = session.get('user_id')
  if user_id in all_session:
    return app.send_static_file("index.html")
  else:
    return redirect(url_for('login'))


@app.route('/sendRequest')
def sendRequest():
  global all_session
  user_id = session.get('user_id')

  if user_id in all_session:
    while not (all_session[user_id]["status_updated"] or 
      (all_session[user_id]["current_board"] is not None \
      and all_session[user_id]["current_valid_moves"] is not None \
      and all_session[user_id]["current_player"] is not None)
      ):
      sleep(0.2)
    all_session[user_id]["status_updated"] = False
    return json.dumps(boardToJsonCheckers(all_session[user_id]["current_board"], all_session[user_id]["current_valid_moves"], all_session[user_id]["current_player"]))
  else:
    return redirect(url_for('login'))



# Получаем Message от index.html
# Получаем доску, делаем шаг и отправляем доску на js
@app.route('/getRequest', methods=['POST'])
def getRequest():
  global all_session
  user_id = session.get('user_id')

  if user_id in all_session:
    result = request.get_json()
    if "reset" in result:
      all_session[user_id]["reply"]["a"] = result["reset"]
    elif "changeSide" in result:
      all_session[user_id]["reply"]["a"] = result["changeSide"]
    else:
      for i in all_session[user_id]["current_valid_moves"]:
        if all_session[user_id]["current_valid_moves"][i][0] == result["move_from"] \
          and all_session[user_id]["current_valid_moves"][i][1] == result["move_to"]:
          all_session[user_id]["reply"]["a"] = i
          break

    all_session[user_id]["response_filled"] = True
    while not all_session[user_id]["status_updated"]:
      sleep(0.2)
    all_session[user_id]["status_updated"] = False
    return json.dumps(boardToJsonCheckers(all_session[user_id]["current_board"], all_session[user_id]["current_valid_moves"], all_session[user_id]["current_player"]))
  else:
    return redirect(url_for('login'))



@app.route('/login', methods=['GET', 'POST'])
def login():
  global all_session

  if request.method == 'POST':
    # создаем сессию
    # присваиваем рандомный номер нашей сессии
    user_id = random.randint(1, 10000)
    session['user_id'] = user_id
    # инициализируем окружения для этой сессии
    all_session[user_id] = {}
    all_session[user_id]["status_updated"] = False
    all_session[user_id]["current_valid_moves"] = None
    all_session[user_id]["current_player"] = None
    all_session[user_id]["current_board"] = None
    all_session[user_id]["game_exit"] = False


    all_session[user_id]["response_filled"] = False
    all_session[user_id]["reply"] = dict(pi=None, a=None, checkers_V=0)

 

    game_thread = start_new_thread(init_observation,((user_id, )))
    return redirect(url_for('main'))

  return app.send_static_file("login.html")





@app.route('/logout', methods=['GET', 'POST'])
def logout():
  global all_session
  # удаляем сессию
  user_id = session.get('user_id')
  # del session['user_id']
  # if user_id in all_session:
  #   del all_session[user_id]

  return redirect(url_for('main'))


if __name__ == '__main__':
  app.run(host='188.163.246.46', debug=True)
