var connection;
var lcd;
var lastArray;

function setCharAt(str, index, chr) {
  if (index > str.length - 1) return str;
  return str.substring(0, index) + chr + str.substring(index + 1);
}

// New Display content received from safe
function NewLcdContent(e) {
  data = e.data;
  for (var i = 0; i < data.length; i++) {
    code = data.charCodeAt(i);
    if (code <= 2) {
      // scrollbar
      data = setCharAt(data, i, "|");
    } else if (code <= 6) {
      // wifi signal strength
      data = setCharAt(data, i, " ");
    }
  }

  var s = data.split("\n");
  if(s[4] != $("#remoteLcd").attr("configAvailable")) {
    // Most likely the configuration server was turned on or off ==> index.html changed!
    location.reload();
  }

  $("#remoteLcdFrame").attr("backlight", s[5]);

  $("#remoteLcd").find("*").attr("disabled", false);
  lcd.writeString({ string: s[0], offset: 0 });
  lcd.writeString({ string: s[1], offset: 20 });
  lcd.writeString({ string: s[2], offset: 40 });
  lcd.writeString({ string: s[3], offset: 60 });
}

// Timer which monitors the websocket to the safe
function refreshTimer() {
  if (connection == null || connection.readyState == 3) {
    try {
      // connection closed, try reconnecting
      connection = new WebSocket("ws://" + window.location.hostname + "/ws");
      connection.onmessage = NewLcdContent;
    } catch {}

    // grey out the display
    $("#remoteLcd").find("*").attr("disabled", true);

    lcd.clearScreen();
    lcd.writeString({ string: "Connecting to Safe!", offset: 20 });
    setTimeout(refreshTimer, 1000);
    return;
  } else if (connection.readyState == 1) {
    // connection alive, just request LCD content
    connection.send("L");
    setTimeout(refreshTimer, 100);
    return;
  }

  // connection is closed or built up, just wait.
  $("#remoteLcd").find("*").attr("disabled", true);
  setTimeout(refreshTimer, 100);
}

// Inject html for the remote control display as soon as the page is loaded
document.addEventListener("DOMContentLoaded", function () {
  var container = document.getElementById("remoteLcd");
  container.innerHTML =
    '<div class="centerContent">' +
    '  <div id="remoteLcdFrame" class="lcd-container" backlight="true"></div>' +
    "</div>" +
    '<div class="centerContent">' +
    '  <button class="encoderButton" type="button" onclick="if (connection.readyState == 1) { connection.send(\'C\');}"><img src="rotate.svg" style="transform: scaleX(-1);"></button>' +
    '  <button class="encoderButton" type="button" onclick="if (connection.readyState == 1) { connection.send(\'B\');}">Click</button>' +
    '  <button class="encoderButton" type="button" onclick="if (connection.readyState == 1) { connection.send(\'A\');}"><img src="rotate.svg"></button>' +
    "</div>";

  lcd = new LCD({
    elem: document.getElementById("remoteLcdFrame"),
    rows: 4,
    columns: 20,
    pixelSize: 2,
    pixelColor: "#FFF",
  });

  refreshTimer();
});
