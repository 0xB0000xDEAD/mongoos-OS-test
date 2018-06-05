load("api_config.js");
load("api_events.js");
load("api_gpio.js");
load("api_mqtt.js");
load("api_net.js");
load("api_sys.js");
load("api_timer.js");
load("api_esp32.js");
load("api_spi.js");

let spi = SPI.get();
let spi_param = {
  // Which CS line to use, 0, 1 or 2. use -1 to not assert any CS
  // during transaction, it is assumed to be done externally.
  // Note: this is not a GPIO number, mapping from cs to GPIO is set in
  // the device configuration.
  cs: 0,

  // Mode, 0-3. This controls clock phase and polarity.
  mode: 0,

  // Clock frequency to use. 0 means don't change.
  freq: 2000000,

  // Half-duplex transaction parameters
  // hd: {
  //   // A string with data to transmit. If undefined, no data is transmitted.
  //   tx_data: "000111111111110100111111111111111110",

  //   // Number of dummy bytes to wait for. If undefined, 0 is assumed.
  //   // dummy_len: 1,

  //   // Number of bytes to read.
  //   rx_len: 3
  // }

  // Full-duplex transaction parameters
  fd: {
    // A string with data to transmit. Equal number of bytes will be read.
    tx_data: "placeholder"
  }
};

let tempOffset = 25;

function convTemp() {
  let temp = 5 / 9 * (ESP32.temp() - 32) - tempOffset;
  // return temp.toFixed(1); // no support for String library
  return temp;
}

load("api_arduino_ssd1306.js");

// let led = Cfg.get("pins.led");
// let button = Cfg.get("pins.button");
// let topic = "/devices/" + Cfg.get("device.id") + "/events";
// print("LED GPIO:", led, "button GPIO:", button);

// Initialize Adafruit_SSD1306 library (I2C)
let d = Adafruit_SSD1306.create_i2c(
  13 /* !!! the original pin 4 can't be used for reset cause is already on clk */ /* RST GPIO */,
  Adafruit_SSD1306.RES_128_64
);
// Initialize the display. !!! address is 0x3C
d.begin(Adafruit_SSD1306.SWITCHCAPVCC, 0x3c, true /* reset */);
d.display();
let i = 0;

let showStr = function(d, str) {
  d.clearDisplay();
  // d.setTextSize(2);
  d.setTextWrap(true);
  d.setTextColor(Adafruit_SSD1306.WHITE);
  // d.setCursor(d.width() / 4, d.height() / 4);
  d.setCursor(5, 5);
  d.write(str);
  d.display();
};

// let getInfo = function() {
//   return JSON.stringify({
//     total_ram: Sys.total_ram(),
//     free_ram: Sys.free_ram()
//   });
// };

// toggle relay every 5 seconds
GPIO.set_mode(15, GPIO.MODE_OUTPUT);
// Timer.set(
//   5000 /* 1 sec */,
//   Timer.REPEAT,
//   function() {
//     print("toggling output...");

//     let value = GPIO.toggle(15);
//     // print(value ? "Tick" : "Tock", "uptime:", Sys.uptime(), getInfo());
//   },
//   null
// );

// Publish to MQTT topic on a button press. Button is wired to GPIO pin 0
// GPIO.set_button_handler(button, GPIO.PULL_UP, GPIO.INT_EDGE_NEG, 20, function() {
//   let message = getInfo();
//   let ok = MQTT.pub(topic, message, 1);
//   print('Published:', ok, topic, '->', message);
// }, null);

let encInit = ffi("void encInit(int, int, int)");
encInit(14, 12, 16); // a, btn, b

let getCount = ffi("int getCount()");

// Timer.set(
//   1000 /* milliseconds */,
//   Timer.REPEAT,
//   function() {
//     showStr(
//       d,
//       // "ram usage is: \n" +
//       //   JSON.stringify(getInfo()) +
//       // "\ntick is: " +
//       // JSON.stringify(i) +
//       "Temp:" + JSON.stringify(convTemp())
//       // "\ncount is: " +
//       // JSON.stringify(getIncrement())
//     );
//     i++; //increment tick print
//   },
//   null
// );

let test_js = ffi("int test()");

Timer.set(
  100,
  Timer.REPEAT,
  function() {
    showStr(
      d,
      "\n\nEncoder value is:\n\n        " + JSON.stringify(getCount() / 4)
    );
  },
  null
);

// GPIO.set_mode(11, GPIO.MODE_OUTPUT);
let lut = [
  "\x00",
  "\x01",
  "\x02",
  "\x04",
  "\x08",
  "\x10",
  "\x20",
  "\x40",
  "\x80",
  "\x01\x00",
  "\x02\x00",
  "\x04\x00",
  "\x08\x00",
  "\x10\x00",
  "\x20\x00",
  "\x40\x00"
];

let lut2 = [
  "\x00\x00",
  "\x00\x01",
  "\x00\x02",
  "\x00\x04",
  "\x00\x08",
  "\x00\x10",
  "\x00\x20",
  "\x00\x40",
  "\x00\x80",
  "\x01\x00",
  "\x02\x00",
  "\x04\x00",
  "\x08\x00",
  "\x10\x00",
  "\x20\x00",
  "\x40\x00"
];

let tick = 0;

// Timer.set(
//   200,
//   Timer.REPEAT,
//   function() {
//     if (tick === 16) {
//       tick = 0;
//     }
//     if (tick < 9) {
//       spi_param.fd.tx_data = "\x00" + lut[tick];
//       print(SPI.runTransaction(spi, spi_param));
//       tick++;
//     } else {
//       spi_param.fd.tx_data = lut[tick];
//       print(SPI.runTransaction(spi, spi_param));
//       tick++;
//     }
//     // GPIO.toggle(11);
//   },
//   null
// );

// map helper
let map = function(x, y, X, Y, input) {
  let slope = (Y - X) / (y - x);
  return 0 + slope * (input - 0);
};

Timer.set(
  200,
  Timer.REPEAT,
  function() {
    let ledPos = map(0, 16, 0, 16, getCount() / 4);
    // print(ledPos);
    // spi_param.fd.tx_data = lut2[ledPos];
    spi_param.fd.tx_data = 0x8000;

    SPI.runTransaction(spi, spi_param);
    
    let string = "paperino";
    print(string.charCodeAt(0)); //works

    // button detection
    if (1) {
    }
  },
  null
);

//If the encoder switch is pushed, this will turn on the bottom LED.  The bottom LED is turned
//   //on by 'OR-ing' the current display with 0x8000 (1000000000000000 in binary)
//   if (!digitalRead(ENC_SW))
//   {
//     digitalWrite(LE,LOW);
//     shiftOut(SDI,CLK,MSBFIRST,((sequence[sequenceNumber][scaledCounter]|0x8000) >> 8));
//     shiftOut(SDI,CLK,MSBFIRST,sequence[sequenceNumber][scaledCounter]);
//     digitalWrite(LE,HIGH);
//   }

Timer.set(
  200,
  Timer.REPEAT,
  function() {
    let btn;
    if (btn) {
    }
  },
  null
);

// // Monitor network connectivity.
// Event.addGroupHandler(
//   Net.EVENT_GRP,
//   function(ev, evdata, arg) {
//     let evs = "???";
//     if (ev === Net.STATUS_DISCONNECTED) {
//       evs = "DISCONNECTED";
//     } else if (ev === Net.STATUS_CONNECTING) {
//       evs = "CONNECTING";
//     } else if (ev === Net.STATUS_CONNECTED) {
//       evs = "CONNECTED";
//     } else if (ev === Net.STATUS_GOT_IP) {
//       evs = "GOT_IP";
//     }
//     print("== Net event:", ev, evs);
//   },
//   null
// );
