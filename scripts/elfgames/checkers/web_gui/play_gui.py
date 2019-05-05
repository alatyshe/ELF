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

from datetime import datetime, timedelta 
from time import sleep
from _thread import start_new_thread
from CheckersGui import boardToJsonCheckers, getValidMoves
from rlpytorch import Evaluator, load_env

from flask import Flask, session, redirect, url_for, request, render_template
import json

num_games = 1

additional_to_load = {
  'evaluator': (
    Evaluator.get_option_spec(),
    lambda object_map: Evaluator(object_map, stats=None)),
}
env = load_env(
  os.environ,
  overrides={
    'num_games': num_games,
    'greedy': True,
    'T': 1,
    'additional_labels': ['checkers_aug_code', 'checkers_move_idx'],
  },
  additional_to_load=additional_to_load)

all_sessions = {}

for game_id in range(num_games):  
  all_sessions[game_id] = {}
  game_pass = random.randint(1, 10000000)
  all_sessions[game_id]["game_pass"] = game_pass
  all_sessions[game_id]["observation_updated"] = False
  all_sessions[game_id]["action_filled"] = False
  all_sessions[game_id]["valid_moves"] = None
  all_sessions[game_id]["curr_player"] = None
  all_sessions[game_id]["board"] = None
  all_sessions[game_id]["free_session"] = True






def init_observation():
  global env
  evaluator = env['evaluator']
  GC = env["game"].initialize()
  model = env["model_loaders"][0].load_model(GC.params)

  mi = env['mi']

  if "actor" not in mi:
    mi.add_model("actor", model)
  else:
    mi.update_model("actor", model)

  mi["actor"].eval()

  # тут описание действий игрока(человека), если у actor_black нужно просто 
  # отправить observation на оценку нейронки, то в данном случае мы отправляем
  # сперва доску и валидные шаги и пр инфу на front-end ожидая ее заполнения,
  # после - передаем заполненные данные в фреймворк.
  def human_actor(batch):
    global all_sessions
    for i in batch["game_idx"]:
      game_idx = i.item()

      res = dict(pi=None, a=-100, checkers_V=0)

      if all_sessions[game_idx]["action_filled"]:
        all_sessions[game_idx]["action_filled"] = False
        res["a"] = all_sessions[game_idx]["action"]
      elif all_sessions[game_idx]["board"] != batch.GC.getGame(game_idx).getBoard():
        all_sessions[game_idx]["board"] = batch.GC.getGame(game_idx).getBoard()
        all_sessions[game_idx]["curr_player"] = batch.GC.getGame(game_idx).getCurrentPlayer()
        all_sessions[game_idx]["valid_moves"] = getValidMoves(batch, game_idx)
        all_sessions[game_idx]["observation_updated"] = True


      return res

  def actor_black(batch):
    return evaluator.actor(batch)

  evaluator.setup(sampler=env["sampler"], mi=mi)

  GC.reg_callback_if_exists("checkers_actor_white", human_actor)
  GC.reg_callback_if_exists("checkers_actor_black", actor_black)

  GC.start()
  GC.GC.getClient().setRequest(
    mi["actor"].step, -1, -1)

  evaluator.episode_start(0)

  while True:
    GC.run()
  GC.stop()



















app = Flask(__name__)
app.secret_key = 'SECRET_KEY'


@app.route('/', methods=['GET', 'POST'])
def main():
  # проверяем на наличие сессии
  game_id = session.get('game_id')
  game_pass = session.get('game_pass')

  if game_id in all_sessions and all_sessions[game_id]['game_pass'] == game_pass:
    return app.send_static_file("index.html")
  else:
    return redirect(url_for('login'))


@app.route('/login', methods=['GET', 'POST'])
def login():
  global all_sessions

  session.clear()
  # если нажали кноку login
  if request.method == 'POST':
    # Освобождаем сессии если время вышло
    for i in range(num_games):
      if not all_sessions[i]["free_session"]:
        delta = datetime.now() - all_sessions[i]["last_move"]
        if delta.seconds > 60 * 20:
          game_pass = random.randint(1, 10000000)
          all_sessions[i]["game_pass"] = game_pass
          all_sessions[i]["free_session"] = True
          all_sessions[i]["action"] = -1
          all_sessions[i]["action_filled"] = True

          all_sessions[i]["observation_updated"] = False
          all_sessions[i]["valid_moves"] = None
          all_sessions[i]["curr_player"] = None
          all_sessions[i]["board"] = None

    for i in range(num_games):
      if all_sessions[i]['free_session'] == True: 
        all_sessions[i]['free_session'] = False
        all_sessions[i]["last_move"] = datetime.now()
        game_pass = random.randint(1, 10000000)
        all_sessions[i]["game_pass"] = game_pass
        session['game_pass'] = game_pass
        session['game_id'] = i
        break

    return redirect(url_for('main'))
  return app.send_static_file("login.html")


@app.route('/logout', methods=['GET', 'POST'])
def logout():
  global all_sessions

  # удаляем сессию
  game_id = session.get('game_id')
  game_pass = session.get('game_pass')

  session.clear()
  # del session['game_pass']
  if game_id in all_sessions and all_sessions[game_id]['game_pass'] == game_pass:
    all_sessions[game_id]['free_session'] = True
    all_sessions[game_id]['game_pass'] = random.randint(1, 10000000)
  return redirect(url_for('main'))












@app.route('/sendRequest')
def sendRequest():
  global all_sessions

  game_id = session.get('game_id')
  game_pass = session.get('game_pass')


  if game_id in all_sessions and all_sessions[game_id]['game_pass'] == game_pass:

    while not (all_sessions[game_id]["observation_updated"] or 
      (all_sessions[game_id]["board"] is not None
      and all_sessions[game_id]["valid_moves"] is not None
      and all_sessions[game_id]["curr_player"] is not None)
      ):
      sleep(0.2)

    all_sessions[game_id]["observation_updated"] = False
    return json.dumps(
        boardToJsonCheckers(all_sessions[game_id]["board"],
          all_sessions[game_id]["valid_moves"],
          all_sessions[game_id]["curr_player"],
          game_id))
  else:
    return redirect(url_for('logout'))










# Получаем Message от index.html
# Получаем доску, делаем шаг и отправляем доску на js
@app.route('/getRequest', methods=['POST'])
def getRequest():
  global all_sessions

  game_id = session.get('game_id')
  game_pass = session.get('game_pass')

  if game_id in all_sessions and all_sessions[game_id]['game_pass'] == game_pass:
    result = request.get_json()

    delta = datetime.now() - all_sessions[game_id]["last_move"]

    if delta.seconds > 60 * 20:
      all_sessions[game_id]["free_session"] = True
      all_sessions[game_id]["action"] = -1
      all_sessions[game_id]["action_filled"] = True
      all_sessions[game_id]["game_pass"] = random.randint(1, 10000000)
      del session['game_pass']
      return app.send_static_file("login.html")

    elif "reset" in result:
      all_sessions[game_id]["action"] = result["reset"]
    elif "changeSide" in result:
      all_sessions[game_id]["action"] = result["changeSide"]
    else:
      for i in all_sessions[game_id]["valid_moves"]:
        if all_sessions[game_id]["valid_moves"][i][0] == result["move_from"] \
          and all_sessions[game_id]["valid_moves"][i][1] == result["move_to"]:
          all_sessions[game_id]["action"] = i
          break

    all_sessions[game_id]["last_move"] = datetime.now()
    all_sessions[game_id]["action_filled"] = True
    
    while not all_sessions[game_id]["observation_updated"]:
      sleep(0.2)

    all_sessions[game_id]["observation_updated"] = False
    return json.dumps(
            boardToJsonCheckers(all_sessions[game_id]["board"], \
              all_sessions[game_id]["valid_moves"], \
              all_sessions[game_id]["curr_player"],
              game_id))
  else:
    return redirect(url_for('logout'))










if __name__ == '__main__':
  # инициализируем 1 экземплар elf для нашей сессии
  start_new_thread(init_observation,(()))
  app.run(host='188.163.246.34', debug=True)

