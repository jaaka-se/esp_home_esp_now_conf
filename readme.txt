# esphome commands
sudo chmod 777 /dev/ttyUSB0

docker run --rm -v "${PWD}":/config --device=/dev/ttyUSB0 -it esphome/esphome  room2.yaml

docker run --rm -v "${PWD}":/config  -it esphome/esphome run room2.yaml

docker run --rm -v "${PWD}":/config  -it esphome/esphome run room2.yaml

docker run --rm -v "${PWD}":/config -it esphome/esphome dashboard roomnow1.yaml

docker run --rm  --net=host -v "${PWD}":/config -it esphome/esphome dashboard ./

