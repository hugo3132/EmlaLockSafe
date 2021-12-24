class MenuItem {
  constructor(text) {
    this.text = text;
    this.showPosition = 0;
    this.scrollForwards = false;
  }

  /**
   * @brief moves the string by one position if necessary
   *
   * @param maxLength maximum number of characters which should be displayed
   */
  animationTick(maxLength) {
    if (this.text.length > maxLength) {
      if (this.scrollForwards) {
        if (this.text.length - this.showPosition <= maxLength) {
          this.scrollForwards = false;
        }
        else {
          this.showPosition++;
        }
      }
      else {
        if (this.showPosition == 0) {
          this.scrollForwards = true;
        }
        else {
          this.showPosition--;
        }
      }
    }
  }

  /**
   * @brief redraws the text
   *
   * @param display pointer to the lcd to which the text should be sent
   * @param maxLength maximum number of characters which should be displayed
   * @param offset position where to write to
   */
  show(display, maxLength, offset) {
    var s = "";
    if (this.text.length <= maxLength) {
      s = this.text;
      for (var i = this.text.length; i < maxLength; i++) {
        s = s + " ";
      }
    }
    else {
      s = this.text.substring(this.showPosition, this.showPosition + maxLength);
    }
    display.writeString({string: s, offset: offset});
  }
}

class MenuView {
  constructor(divId, title, menuItems) {
    var mainDiv = document.getElementById(divId);
    mainDiv.align = 'center';
    mainDiv.innerHTML = '<div id="' + divId + 'XXXX" class="lcd-container" style="background-color:#33F;"></div>';

    this.lcd = new LCD({
      elem: document.getElementById(divId+ 'XXXX'),
      rows: Math.max(4, menuItems.length + 1),
      columns: 20,
      pixelSize: 2,
      pixelColor: "#FFF"});
    
    this.lcd.writeString({string: title, offset: 0});

    this.menuItems = [];
    menuItems.forEach(element => this.menuItems.push(new MenuItem(element)));
    this.tick();
  }

  tick() {
    // We have three lines left for the menu items
    for (var i = 0; i < Math.max(3, this.menuItems.length); i++) {
      if(i < this.menuItems.length) {
        // Is the item selected?
        if (i == 0) {
          this.lcd.writeString({string: ">", offset: (i+1)*20});
        }
        else {
          this.lcd.writeString({string: " ", offset: (i+1)*20});
        }

        // The animation must be updated
        this.menuItems[i].animationTick(19);

        // draw the menu item
        this.menuItems[i].show(this.lcd, 19, (i+1)*20 + 1);
      }
      else {
        // Not enough items to be displayed, just clear the line
        if (fullRedraw) {
          s = "";
          for (j = 0; j < 19 + 1; j++) {
            s = s + " "
          }
          this.lcd.writeString({string: s, offset: (i+1)*20});
        }
      }
    }
  }
}

var menus = [];
setTimeout(tickLcds, 500);

function tickLcds() {
  menus.forEach(element => element.tick());
  setTimeout(tickLcds, 500);
}

function zeroPad(num, places) {
  var zero = places - num.toString().length + 1;
  return Array(+(zero > 0 && zero)).join("0") + num;
}