# water_cooler_buddy

### About the Project:
Electrical Engineering and Computer Science (EECS) senior design project at University of California Irvine (UCI). Cooperated with California Institute for Telecommunication and Information Technology (Calit2), California Plugload Research Center (CalPlug), funded by Undergraduated Research Opportunities Program (UROP), we have sucessfully designed, developed and manufactured the first smart water cooler dispenser. In addition, we developed the mixing function. Users will not only be able to get just hot or just cold water but will be able to select a specific temperature of the water. As a result, this machine will cut down the anual electricity bill by 50% and will be the first of its kind to offer the mixing function.

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

*Note on uploading code: you may need to do the following step to upload code in ubuntu:*
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

### Who are we:
Hugh Dang https://www.linkedin.com/in/hugh-hung-dang-13315497/  
Binh Nguyen https://www.linkedin.com/in/binh-edward-nguyen  
Duy Nguyen www.linkedin.com/in/duy-nguyen-12584ab5  
Phat Quach https://www.linkedin.com/in/phat-quach-48678b107  
