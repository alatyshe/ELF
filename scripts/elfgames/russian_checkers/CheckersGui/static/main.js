

function DisplayBoard(boardData){
  var fieldClass;

  // чистим код html
  $("#chessboard").empty();
  $("#onLoad").css("z-index", "-1")
  $("#onLoad").css("background-color", "rgba(10, 10, 10, 0.0)")

  if(boardData["rotate"] == 1) {
    $("#chessboard").css("transform","rotate(180deg)");
  } else {
    $("#chessboard").css("transform","rotate(0deg)");
  }

  table = "<tbody>";
  for (y_ = 0; y_ < 8; y_++) {
    table += "<tr class='y_" + y_ + "'>";
    for (x_ = 0; x_ < 8; x_++) {
      fieldClass = "";
      if (boardData[y_] != undefined && boardData[y_][x_] != undefined) {
        fieldClass = boardData[y_][x_];
      }
      table += "<td class='y_" + y_
        + " x_" + x_
        + "'>  <span class='"
        + fieldClass + "'></span></td>";
    }
    table += "</tr>";
  }
  table += "</tbody>";
  // Ищем борду на нашей html странице
  var board = $("#chessboard");
  
  // добавляем борду на страницу
  if (document.querySelectorAll('#resetGame').length == 0) {
    var container = $("#container");
    button = "<div class='buttonContainer'>";
    button +="<button id='resetGame' onclick='resetGame()'>Reset Game</button>";
    button += "<button id='changeSide' onclick='changeSide()'>Change Side</button>";
    // button += "<button id='endSession' onclick='endSession()'>End Session</button>";
    button += "</div>";
    container.append(button);
  }
  board.append(table);
}

function sendBoard(info){
  // отправляем это все на falsk и ждем следующей борды
  $("#onLoad").css("z-index", "3")
  $("#onLoad").css("background-color", "rgba(10, 10, 10, 0.4)")

  jQuery.ajax ({
    url: "getRequest",
    type: "POST",
    data: JSON.stringify(info),
    dataType: "json",
    contentType: "application/json; charset=utf-8",
    
    error: function(){ 
        alert("Oops, server error. Game was lost."); 
    },

    success: function(data){
      console.log("Data successfully Recieved");
      console.log(JSON.stringify(data));
      DisplayBoard(data);
      subscribeEvents($('#chessboard'), data);
    }
  });
}

function endSession() {
  window.location = "logout";
}



function changeSide(){
  var result = {};
  result["changeSide"] = -2;
  sendBoard(result)
}


function resetGame(){
  var result = {};
  result["reset"] = -1;
  sendBoard(result)
}

function getCoordFromClasses(classes){
  var x = 0;
  var y = 0;

  for (var index = 0; index < classes.length; ++index) {
    if (classes[index].startsWith("y_")){
      x = classes[index].replace("y_", "");
    }
    if (classes[index].startsWith("x_")){
      y = classes[index].replace("x_", "");
    }
  }
  return [x, y];
}


// ивенты для ходов 
function subscribeEvents(board, data){
  console.log("subscribeEvents : ", data["valid_moves"])

  $("html").click(
    function () {
      $("td").removeClass("move-possible move-selected");
    });

  board.find("td").click(
    function () {
      var move_from;
      var move_to;
      var coord;
      var result = {};

      if ($(this).hasClass("move-possible")) {
        // move_from = getCoordFromClasses($(".move-selected").attr('class').split(/\s+/));
        // move_to = getCoordFromClasses($(this).attr('class').split(/\s+/));
        move_from = $(".move-selected").attr('class').replace(" move-selected", "")
        move_to = $(this).attr('class').replace(" move-possible", "")

        result["move_from"] = move_from;
        result["move_to"] = move_to;
        // DisplayBoard(nextmoves[index].next_state);
        // subscribeEvents($("#chessboard"));
        console.log(result)
        sendBoard(result);
      } else {
        console.log("shit : ")

        board.find("td").removeClass("move-selected move-possible");
        coord = getCoordFromClasses($(this).attr('class').split(/\s+/));

        str_move_from = "y_" + coord[0] + " x_" + coord[1];
        if(str_move_from in data["valid_moves"]) {
          $(this).addClass("move-selected");
          // board.find(".y_" + coord[0] + ".x_" + coord[1]).addClass("move-selected");

          moves = data["valid_moves"][str_move_from];
          for (var i = 0; i < moves.length; i++) {
            move_possible = getCoordFromClasses(moves[i].split(/\s+/));
            board.find(".y_" + move_possible[0] + ".x_" + move_possible[1]).addClass("move-possible");
          }
        } else {
          board.find("td").removeClass("move-selected move-possible");
        }
      }
      event.stopPropagation();
    });
  

  board.find("td").hover(
    function() {
      coord = getCoordFromClasses($(this).attr('class').split(/\s+/));

      str_move_from = "y_" + coord[0] + " x_" + coord[1];
      if(str_move_from in data["valid_moves"]) {
        moves = data["valid_moves"][str_move_from];
        for (var i = 0; i < moves.length; i++) {
          move_possible = getCoordFromClasses(moves[i].split(/\s+/));
          board.find(".y_" + move_possible[0] + ".x_" + move_possible[1]).addClass("highlight-hover");
        }
      }
    },
    function() {
      $("#chessboard").find("td").removeClass("highlight-hover");
    } 
  )
}

board = {};

// $( document ).onload(function() {
//   console.log("fuck")
// });


$( document ).ready(function() {


  $.getJSON("/sendRequest", function(data) {
    DisplayBoard(data);
    subscribeEvents($("#chessboard"), data);
  });
});