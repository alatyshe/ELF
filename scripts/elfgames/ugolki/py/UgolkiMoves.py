#!/usr/bin/env python

""" 
  Way to collect all possible moves for our board,
  becouse the board is represented by six numbers
  and each turn represent by a number. 
  Also take a look at HashAllMoves.h
"""
def get_all_moves_ugolki():
  all_moves = [[0, 1],
  [1, 2],
  [2, 3],
  [3, 4],
  [4, 5],
  [5, 6],
  [6, 7],
  [8, 9],
  [9, 10],
  [10, 11],
  [11, 12],
  [12, 13],
  [13, 14],
  [14, 15],
  [16, 17],
  [17, 18],
  [18, 19],
  [19, 20],
  [20, 21],
  [21, 22],
  [22, 23],
  [24, 25],
  [25, 26],
  [26, 27],
  [27, 28],
  [28, 29],
  [29, 30],
  [30, 31],
  [32, 33],
  [33, 34],
  [34, 35],
  [35, 36],
  [36, 37],
  [37, 38],
  [38, 39],
  [40, 41],
  [41, 42],
  [42, 43],
  [43, 44],
  [44, 45],
  [45, 46],
  [46, 47],
  [48, 49],
  [49, 50],
  [50, 51],
  [51, 52],
  [52, 53],
  [53, 54],
  [54, 55],
  [56, 57],
  [57, 58],
  [58, 59],
  [59, 60],
  [60, 61],
  [61, 62],
  [62, 63],
  [1, 0],
  [2, 1],
  [3, 2],
  [4, 3],
  [5, 4],
  [6, 5],
  [7, 6],
  [9, 8],
  [10, 9],
  [11, 10],
  [12, 11],
  [13, 12],
  [14, 13],
  [15, 14],
  [17, 16],
  [18, 17],
  [19, 18],
  [20, 19],
  [21, 20],
  [22, 21],
  [23, 22],
  [25, 24],
  [26, 25],
  [27, 26],
  [28, 27],
  [29, 28],
  [30, 29],
  [31, 30],
  [33, 32],
  [34, 33],
  [35, 34],
  [36, 35],
  [37, 36],
  [38, 37],
  [39, 38],
  [41, 40],
  [42, 41],
  [43, 42],
  [44, 43],
  [45, 44],
  [46, 45],
  [47, 46],
  [49, 48],
  [50, 49],
  [51, 50],
  [52, 51],
  [53, 52],
  [54, 53],
  [55, 54],
  [57, 56],
  [58, 57],
  [59, 58],
  [60, 59],
  [61, 60],
  [62, 61],
  [63, 62],
  [0, 8],
  [1, 9],
  [2, 10],
  [3, 11],
  [4, 12],
  [5, 13],
  [6, 14],
  [7, 15],
  [8, 16],
  [9, 17],
  [10, 18],
  [11, 19],
  [12, 20],
  [13, 21],
  [14, 22],
  [15, 23],
  [16, 24],
  [17, 25],
  [18, 26],
  [19, 27],
  [20, 28],
  [21, 29],
  [22, 30],
  [23, 31],
  [24, 32],
  [25, 33],
  [26, 34],
  [27, 35],
  [28, 36],
  [29, 37],
  [30, 38],
  [31, 39],
  [32, 40],
  [33, 41],
  [34, 42],
  [35, 43],
  [36, 44],
  [37, 45],
  [38, 46],
  [39, 47],
  [40, 48],
  [41, 49],
  [42, 50],
  [43, 51],
  [44, 52],
  [45, 53],
  [46, 54],
  [47, 55],
  [48, 56],
  [49, 57],
  [50, 58],
  [51, 59],
  [52, 60],
  [53, 61],
  [54, 62],
  [55, 63],
  [8, 0],
  [9, 1],
  [10, 2],
  [11, 3],
  [12, 4],
  [13, 5],
  [14, 6],
  [15, 7],
  [16, 8],
  [17, 9],
  [18, 10],
  [19, 11],
  [20, 12],
  [21, 13],
  [22, 14],
  [23, 15],
  [24, 16],
  [25, 17],
  [26, 18],
  [27, 19],
  [28, 20],
  [29, 21],
  [30, 22],
  [31, 23],
  [32, 24],
  [33, 25],
  [34, 26],
  [35, 27],
  [36, 28],
  [37, 29],
  [38, 30],
  [39, 31],
  [40, 32],
  [41, 33],
  [42, 34],
  [43, 35],
  [44, 36],
  [45, 37],
  [46, 38],
  [47, 39],
  [48, 40],
  [49, 41],
  [50, 42],
  [51, 43],
  [52, 44],
  [53, 45],
  [54, 46],
  [55, 47],
  [56, 48],
  [57, 49],
  [58, 50],
  [59, 51],
  [60, 52],
  [61, 53],
  [62, 54],
  [63, 55],
  [0, 2],
  [1, 3],
  [2, 4],
  [3, 5],
  [4, 6],
  [5, 7],
  [8, 10],
  [9, 11],
  [10, 12],
  [11, 13],
  [12, 14],
  [13, 15],
  [16, 18],
  [17, 19],
  [18, 20],
  [19, 21],
  [20, 22],
  [21, 23],
  [24, 26],
  [25, 27],
  [26, 28],
  [27, 29],
  [28, 30],
  [29, 31],
  [32, 34],
  [33, 35],
  [34, 36],
  [35, 37],
  [36, 38],
  [37, 39],
  [40, 42],
  [41, 43],
  [42, 44],
  [43, 45],
  [44, 46],
  [45, 47],
  [48, 50],
  [49, 51],
  [50, 52],
  [51, 53],
  [52, 54],
  [53, 55],
  [56, 58],
  [57, 59],
  [58, 60],
  [59, 61],
  [60, 62],
  [61, 63],
  [2, 0],
  [3, 1],
  [4, 2],
  [5, 3],
  [6, 4],
  [7, 5],
  [10, 8],
  [11, 9],
  [12, 10],
  [13, 11],
  [14, 12],
  [15, 13],
  [18, 16],
  [19, 17],
  [20, 18],
  [21, 19],
  [22, 20],
  [23, 21],
  [26, 24],
  [27, 25],
  [28, 26],
  [29, 27],
  [30, 28],
  [31, 29],
  [34, 32],
  [35, 33],
  [36, 34],
  [37, 35],
  [38, 36],
  [39, 37],
  [42, 40],
  [43, 41],
  [44, 42],
  [45, 43],
  [46, 44],
  [47, 45],
  [50, 48],
  [51, 49],
  [52, 50],
  [53, 51],
  [54, 52],
  [55, 53],
  [58, 56],
  [59, 57],
  [60, 58],
  [61, 59],
  [62, 60],
  [63, 61],
  [0, 16],
  [1, 17],
  [2, 18],
  [3, 19],
  [4, 20],
  [5, 21],
  [6, 22],
  [7, 23],
  [8, 24],
  [9, 25],
  [10, 26],
  [11, 27],
  [12, 28],
  [13, 29],
  [14, 30],
  [15, 31],
  [16, 32],
  [17, 33],
  [18, 34],
  [19, 35],
  [20, 36],
  [21, 37],
  [22, 38],
  [23, 39],
  [24, 40],
  [25, 41],
  [26, 42],
  [27, 43],
  [28, 44],
  [29, 45],
  [30, 46],
  [31, 47],
  [32, 48],
  [33, 49],
  [34, 50],
  [35, 51],
  [36, 52],
  [37, 53],
  [38, 54],
  [39, 55],
  [40, 56],
  [41, 57],
  [42, 58],
  [43, 59],
  [44, 60],
  [45, 61],
  [46, 62],
  [47, 63],
  [16, 0],
  [17, 1],
  [18, 2],
  [19, 3],
  [20, 4],
  [21, 5],
  [22, 6],
  [23, 7],
  [24, 8],
  [25, 9],
  [26, 10],
  [27, 11],
  [28, 12],
  [29, 13],
  [30, 14],
  [31, 15],
  [32, 16],
  [33, 17],
  [34, 18],
  [35, 19],
  [36, 20],
  [37, 21],
  [38, 22],
  [39, 23],
  [40, 24],
  [41, 25],
  [42, 26],
  [43, 27],
  [44, 28],
  [45, 29],
  [46, 30],
  [47, 31],
  [48, 32],
  [49, 33],
  [50, 34],
  [51, 35],
  [52, 36],
  [53, 37],
  [54, 38],
  [55, 39],
  [56, 40],
  [57, 41],
  [58, 42],
  [59, 43],
  [60, 44],
  [61, 45],
  [62, 46],
  [63, 47],
  [0, 0]]

  return all_moves