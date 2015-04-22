var GpioStream = require('gpio-stream'),
    button = GpioStream.readable(17),
    AIO = require('adafruit-io'),
    aio = AIO('xxxxxxxxxxx'); // replace xxxxxxxxxxx with your Adafruit IO key

button.pipe(aio.feeds('Button'));
