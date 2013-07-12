Before using do:

sudo apt-get install libusb-1.0-0-dev
sudo apt-get install libftdi-dev

if running client:
place the file 98-usb-probe.rules of /pi/ into /etc/udev/rules.d/ 
place the file autorun.sh in home directory
type "sudo nano .bashrc", go to the last line and add: "sh autorun.sh", save and exit.
