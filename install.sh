apt-get install libusb-1.0-0-dev
apt-get install libftdi-dev
apt-get install libcurl4-openssl-dev

cp client/pi/98-usb-probe.rules /etc/udev/rules.d/
cp client/pi/autorun.sh ~

if grep -q "sh autorun.sh" ~/.bashrc; then
    echo "sh autorun.sh" >> ~/.bashrc
fi
