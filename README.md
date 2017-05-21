# water_cooler_buddy
Electrical Engineering and Computer Science (EECS) senior design project at University of California Irvine (UCI).

### Install (Ubuntu):
```bash
cd Desktop
git clone https://github.com/binhnn1/water_cooler_buddy.git
cd water_cooler_buddy
cp -r libraries/* ../../$YOUR ARDUINO VERSION$/
//$YOUR ARDUINO VERSION e.g. arduino-1.8.0
open water_cooler_buddy.ino in ~/Desktop/water_cooler_buddy
Build and Upload Code
```

*Note on uploading code: you may need to do the following step to upload code in ubuntu:
1. Open up terminal
2. sudo chmod 777 /dev/tty...
3. Enter your password
4. Upload code

### Install (Windows):
```bash
1. Download project from https://github.com/binhnn1/water_cooler_buddy/archive/master.zip
2. Unzip
3. Copy everything from libraries folder to your arduino libraries folder
4. Open water_cooler_buddy.ino
5. Build and Upload Code
```

### Arduino Configuration:
```bash
Board: NodeMCU 0.8(ESP-12 Module)
CPU Frequency: 80Hz
Flash Size: 4M (3M SPIFFS)
Upload Speed: 115200
```
