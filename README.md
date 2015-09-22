#What: Friction Drum docs (written 22th of Sep 2015)
_By Who: LGM UL_
_(Karmen Gostiša and Miha Mohorčič under Matevž Pesek and Matija Marolt )_

Purpose: Recreate Friction Drum in an electronic shape. Do it cheaply, add functionality and present possible use cases

HW demands: 
    Raspberry PI (used B model, but could use newer too)
    MPR121 capacitive touch sensor
        RPi connected to I2C on default (5A) address
    Digital or analog joystick (we used analog with MCP3008 ADC)
        RPi connected to SPI
        Joystick press can be connected to pin 0 on MCP3008
        Joystick X was connected to PIN 1 on MCP3008
        Joystick Y was connected to PIN 2 on MCP3008
    Homemade stick (or however you wish to “interface” with touch sensor)

SW demands: 
    RPi on fresh wheezy install use raspi-config to: 
    Set timezone and locals (so apt-get will work correctly)
    Turn I2C and SPI on (under advanced options)
    Force audio through 3.5 jack (under advanced options)

Installation instructions (can c/p commands): 
```
# install SDL
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install build-essential libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libasound2-dev libjpeg8-dev libtiff5-dev libwebp-dev automake
cd ~
wget https://www.libsdl.org/release/SDL2-2.0.3.tar.gz
tar zxvf SDL2-2.0.3.tar.gz
cd SDL2-2.0.3 && mkdir build && cd build
../configure --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl
make -j 4
sudo make install
sudo ldconfig -v

# install wiringPi
cd ~
git clone git://git.drogon.net/wiringPi
cd wiringPi
git pull origin
./build
sudo ldconfig -v

# get frictionBass and soundtouch
cd ~/Desktop
git clone https://github.com/miha-mohorcic/Lonceni_Bas.git
cd Lonceni_Bas/Code
mkdir output
./build 
# If everything went OK, your program should be running now. 
```

Usage: 
    To start program on boot put this line in rc.local:
```
sudo /home/pi/Desktop/Lonceni_Bas/Code/output/program
```
    before exit.

    Produce a gesture to recreate Friction drum sounds (pull up or down with fingers on the stick).
    Tap on the top to get the TAP sound.

    Press q then enter to  exit the program.
    Press 0 then enter to play Friction drum.
    Press 1 then enter to produce square wave.
    Press 2 then enter to produce sine wave.

CODE:
    Available here: https://github.com/miha-mohorcic/Lonceni_Bas

Some TODOs:
    Add some wiring diagram to docs    
    wiring - default for SPI (MCP3008) and I2C (MPR121) communication on RPi
    Performance improvements ( clipping, delay etc…) 
    Add functionality (Karplus strong not implemented, add maybe beat samples…)
    Refactor code (get rid of build.sh script and finally use MAKE) 
