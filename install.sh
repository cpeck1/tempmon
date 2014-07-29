apt-get install libusb-1.0-0-dev
apt-get install libftdi-dev
apt-get install libcurl4-openssl-dev

chmod 755 tempmon.sh
chmod 755 tempmonDaemon
mv tempmonDaemon /etc/init.d/
update-rc.d tempmonDaemon defaults

mv /etc/udev/rules.d/99-input.rules /etc/udev/rules.d/97-input.rules

cp client/pi/98-usb-probe.rules /etc/udev/rules.d/

