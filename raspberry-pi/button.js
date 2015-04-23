var GpioStream = require('gpio-stream'),
    button = GpioStream.readable(17),
    AIO = require('adafruit-io');

// replace xxxxxxxxxxx with your Adafruit IO key
var AIO_KEY = 'xxxxxxxxxxx';

// aio init
var aio = AIO(AIO_KEY);

// pipe button presses to the button feed
button.pipe(aio.feeds('Button'));

console.log('listening for button presses...');
