# 361 Sensor Project

## Tips for Runnning a "Sketch"
If things aren't uploading, look here!
1. Set board to Teensy 4.0 (in Tools)
2. Ensure a USB port is selected (in Tools)

## Setup
To get started, run these in order!

**Running ldr-sd-example**
1. Upload & open up serial port
2. Make sure pressing breadboard buttons does something 

**Running test-dht.ino**
1. Tools > Manage libraries
    - Search "DHT" - install "DHT sensor library" by "Adafruit" (v 1.4.4)
    - Should be prompted to "Install All" - do it
2. Upload & open up serial port
3. Play with the temp sensor - make sure temp goes up when you hold it

After making sure these two work, you should be able to run `main.ino`!

## TODO
- [x] Temperature seems to be should be in Celcius, but for some reason it's not. Let's fix this!
- [x] If we only want to record every hour of data, we may want to measure every minute so we can store the average of that hour
- [x] I've removed one button (for simplicity). What should the remaining button do in the prototype? Right now it simply prints to the serial monitor, but maybe it should start a new file so we can decide when we want to start recording?
- [ ] Right now, averaging over the hour will start as soon as the program does. Do we want to use the RTC to make sure that we are in sync with the hours in a day? (e.g. 9am-10am as opposed to 9:23am-10:23am)