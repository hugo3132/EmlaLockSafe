EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "EmlaLockSafe Controller Module"
Date "2021-12-10"
Rev "2.0"
Comp "hugo3132"
Comment1 "BSD 2-Clause License"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L DS3231_Module_AzDelivery:DS3231_Module_AzDelivery RTC1
U 1 1 5F3BA624
P 5700 2200
F 0 "RTC1" H 5700 2831 50  0000 C CNN
F 1 "DS3231_Module_AzDelivery" H 5700 2740 50  0000 C CNN
F 2 "DS3231_Module_AzDelivery:DS3231_Module_AzDelivery" H 5700 2200 50  0001 C CNN
F 3 "" H 5700 2200 50  0001 C CNN
	1    5700 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5100 2050 5000 2050
Wire Wire Line
	5100 1950 5000 1950
Wire Wire Line
	5700 2500 5700 2600
Wire Wire Line
	5700 1750 5700 1500
Text GLabel 2750 2850 2    50   Input ~ 0
SDA
Text GLabel 2750 2950 2    50   Input ~ 0
SCL
Text GLabel 5000 1950 0    50   Input ~ 0
SCL
Text GLabel 5000 2050 0    50   Input ~ 0
SDA
$Comp
L Connector_Generic:Conn_01x08 LCD/Encoder_Connecor1
U 1 1 5F3C574A
P 8800 1900
F 0 "LCD/Encoder_Connecor1" H 8880 1892 50  0000 L CNN
F 1 "Conn_01x08" H 8880 1801 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Horizontal" H 8800 1900 50  0001 C CNN
F 3 "~" H 8800 1900 50  0001 C CNN
	1    8800 1900
	1    0    0    -1  
$EndComp
Text GLabel 8450 1800 0    50   Input ~ 0
SDA
Text GLabel 8450 1900 0    50   Input ~ 0
SCL
Wire Wire Line
	8450 1600 8600 1600
Wire Wire Line
	8450 1800 8600 1800
Wire Wire Line
	8600 1900 8450 1900
Wire Wire Line
	8450 2300 8600 2300
Text GLabel 2750 3150 2    50   Input ~ 0
ENC_SW
Text GLabel 2750 3250 2    50   Input ~ 0
ENC_DT
Text GLabel 2750 3350 2    50   Input ~ 0
ENC_CLK
Text GLabel 8450 2000 0    50   Input ~ 0
ENC_CLK
Text GLabel 8450 2100 0    50   Input ~ 0
ENC_DT
Text GLabel 8450 2200 0    50   Input ~ 0
ENC_SW
Wire Wire Line
	8450 2000 8600 2000
Wire Wire Line
	8450 2100 8600 2100
Wire Wire Line
	8450 2200 8600 2200
$Comp
L Connector:Screw_Terminal_01x02 COIL_Connector1
U 1 1 5F3C509F
P 3450 5800
F 0 "COIL_Connector1" H 3530 5792 50  0000 L CNN
F 1 "Screw_Terminal_01x02" H 3530 5701 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 3450 5800 50  0001 C CNN
F 3 "~" H 3450 5800 50  0001 C CNN
	1    3450 5800
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D2
U 1 1 5F3C5EFC
P 2450 6050
F 0 "D2" V 2489 5933 50  0000 R CNN
F 1 "LED" V 2398 5933 50  0000 R CNN
F 2 "LED_THT:LED_D5.0mm" H 2450 6050 50  0001 C CNN
F 3 "~" H 2450 6050 50  0001 C CNN
	1    2450 6050
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R1
U 1 1 5F3C64B7
P 2450 5700
F 0 "R1" H 2380 5654 50  0000 R CNN
F 1 "470" H 2380 5745 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 2380 5700 50  0001 C CNN
F 3 "~" H 2450 5700 50  0001 C CNN
	1    2450 5700
	-1   0    0    1   
$EndComp
$Comp
L Device:R R2
U 1 1 5F3C6BC7
P 2100 6650
F 0 "R2" H 2030 6604 50  0000 R CNN
F 1 "1k" H 2030 6695 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 2030 6650 50  0001 C CNN
F 3 "~" H 2100 6650 50  0001 C CNN
	1    2100 6650
	-1   0    0    1   
$EndComp
$Comp
L Transistor_FET:IRLML2060 Q1
U 1 1 5F3C707A
P 2350 6450
F 0 "Q1" H 2556 6496 50  0000 L CNN
F 1 "IRLML6244" H 2556 6405 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2550 6375 50  0001 L CIN
F 3 "https://www.infineon.com/dgdl/irlml2060pbf.pdf?fileId=5546d462533600a401535664b7fb25ee" H 2350 6450 50  0001 L CNN
	1    2350 6450
	1    0    0    -1  
$EndComp
$Comp
L Diode:1N4148 D1
U 1 1 5F3C9BB3
P 2800 5850
F 0 "D1" V 2754 5929 50  0000 L CNN
F 1 "1N4148" V 2845 5929 50  0000 L CNN
F 2 "Diode_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 2800 5675 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/1N4148_1N4448.pdf" H 2800 5850 50  0001 C CNN
	1    2800 5850
	0    1    1    0   
$EndComp
Wire Wire Line
	2450 5550 2800 5550
Wire Wire Line
	3250 6250 2800 6250
Wire Wire Line
	2450 6250 2450 6200
Wire Wire Line
	2450 5900 2450 5850
Wire Wire Line
	2800 6000 2800 6250
Connection ~ 2800 6250
Wire Wire Line
	2800 6250 2450 6250
Wire Wire Line
	2800 5700 2800 5550
Connection ~ 2800 5550
Wire Wire Line
	2800 5550 3250 5550
Wire Wire Line
	3250 5550 3250 5800
Wire Wire Line
	3250 5900 3250 6250
Connection ~ 2450 6250
$Comp
L power:+3.3V #PWR0101
U 1 1 5F400170
P 1350 850
F 0 "#PWR0101" H 1350 700 50  0001 C CNN
F 1 "+3.3V" H 1365 1023 50  0000 C CNN
F 2 "" H 1350 850 50  0001 C CNN
F 3 "" H 1350 850 50  0001 C CNN
	1    1350 850 
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0102
U 1 1 5F400BED
P 1550 850
F 0 "#PWR0102" H 1550 700 50  0001 C CNN
F 1 "+5V" H 1565 1023 50  0000 C CNN
F 2 "" H 1550 850 50  0001 C CNN
F 3 "" H 1550 850 50  0001 C CNN
	1    1550 850 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 5F40180A
P 1400 1200
F 0 "#PWR0103" H 1400 950 50  0001 C CNN
F 1 "GND" H 1405 1027 50  0000 C CNN
F 2 "" H 1400 1200 50  0001 C CNN
F 3 "" H 1400 1200 50  0001 C CNN
	1    1400 1200
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0104
U 1 1 5F404E22
P 5700 1500
F 0 "#PWR0104" H 5700 1350 50  0001 C CNN
F 1 "+3.3V" H 5715 1673 50  0000 C CNN
F 2 "" H 5700 1500 50  0001 C CNN
F 3 "" H 5700 1500 50  0001 C CNN
	1    5700 1500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5F405E58
P 5700 2600
F 0 "#PWR0105" H 5700 2350 50  0001 C CNN
F 1 "GND" H 5705 2427 50  0000 C CNN
F 2 "" H 5700 2600 50  0001 C CNN
F 3 "" H 5700 2600 50  0001 C CNN
	1    5700 2600
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0106
U 1 1 5F406AA6
P 8450 1600
F 0 "#PWR0106" H 8450 1450 50  0001 C CNN
F 1 "+5V" V 8465 1728 50  0000 L CNN
F 2 "" H 8450 1600 50  0001 C CNN
F 3 "" H 8450 1600 50  0001 C CNN
	1    8450 1600
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5F4070D6
P 8400 1700
F 0 "#PWR0107" H 8400 1450 50  0001 C CNN
F 1 "GND" V 8405 1572 50  0000 R CNN
F 2 "" H 8400 1700 50  0001 C CNN
F 3 "" H 8400 1700 50  0001 C CNN
	1    8400 1700
	0    1    1    0   
$EndComp
Wire Wire Line
	8400 1700 8600 1700
$Comp
L power:+3.3V #PWR0108
U 1 1 5F407EE2
P 8450 2300
F 0 "#PWR0108" H 8450 2150 50  0001 C CNN
F 1 "+3.3V" V 8465 2428 50  0000 L CNN
F 2 "" H 8450 2300 50  0001 C CNN
F 3 "" H 8450 2300 50  0001 C CNN
	1    8450 2300
	0    -1   -1   0   
$EndComp
$Comp
L power:+5V #PWR0109
U 1 1 5F409CA7
P 2800 5500
F 0 "#PWR0109" H 2800 5350 50  0001 C CNN
F 1 "+5V" H 2815 5673 50  0000 C CNN
F 2 "" H 2800 5500 50  0001 C CNN
F 3 "" H 2800 5500 50  0001 C CNN
	1    2800 5500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 5F40A26C
P 2450 6850
F 0 "#PWR0110" H 2450 6600 50  0001 C CNN
F 1 "GND" H 2455 6677 50  0000 C CNN
F 2 "" H 2450 6850 50  0001 C CNN
F 3 "" H 2450 6850 50  0001 C CNN
	1    2450 6850
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 5550 2800 5500
Wire Wire Line
	2450 6650 2450 6850
$Comp
L power:GND #PWR0111
U 1 1 5F4135A7
P 2100 6850
F 0 "#PWR0111" H 2100 6600 50  0001 C CNN
F 1 "GND" H 2105 6677 50  0000 C CNN
F 2 "" H 2100 6850 50  0001 C CNN
F 3 "" H 2100 6850 50  0001 C CNN
	1    2100 6850
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 6450 2100 6450
Wire Wire Line
	2100 6450 2100 6500
Wire Wire Line
	2100 6850 2100 6800
Text GLabel 2050 6450 0    50   Input ~ 0
COIL
Wire Wire Line
	2050 6450 2100 6450
Connection ~ 2100 6450
Text GLabel 1500 2550 0    50   Input ~ 0
COIL
$Comp
L Device:C C1
U 1 1 5F41EBD1
P 2250 4300
F 0 "C1" H 2365 4346 50  0000 L CNN
F 1 "100n" H 2365 4255 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D4.3mm_W1.9mm_P5.00mm" H 2288 4150 50  0001 C CNN
F 3 "~" H 2250 4300 50  0001 C CNN
	1    2250 4300
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C2
U 1 1 5F41F4BF
P 1850 4300
F 0 "C2" H 1968 4346 50  0000 L CNN
F 1 "100µ" H 1968 4255 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D5.0mm_P2.50mm" H 1888 4150 50  0001 C CNN
F 3 "~" H 1850 4300 50  0001 C CNN
	1    1850 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 4450 2050 4450
Wire Wire Line
	1850 4150 2050 4150
$Comp
L power:+5V #PWR0112
U 1 1 5F4246E0
P 2050 4150
F 0 "#PWR0112" H 2050 4000 50  0001 C CNN
F 1 "+5V" H 2065 4323 50  0000 C CNN
F 2 "" H 2050 4150 50  0001 C CNN
F 3 "" H 2050 4150 50  0001 C CNN
	1    2050 4150
	1    0    0    -1  
$EndComp
Connection ~ 2050 4150
Wire Wire Line
	2050 4150 2250 4150
$Comp
L power:GND #PWR0113
U 1 1 5F424C8F
P 2050 4450
F 0 "#PWR0113" H 2050 4200 50  0001 C CNN
F 1 "GND" H 2055 4277 50  0000 C CNN
F 2 "" H 2050 4450 50  0001 C CNN
F 3 "" H 2050 4450 50  0001 C CNN
	1    2050 4450
	1    0    0    -1  
$EndComp
Connection ~ 2050 4450
Wire Wire Line
	2050 4450 1850 4450
Text Notes 1250 7300 0    50   ~ 0
R2 must be "small" because when booting the pull-up resistor of the ESP32 is briefly active and would  unlock the COIL.
$Comp
L ESP32_mini_KiCad_Library:mini_esp32 U1
U 1 1 61B10496
P 2150 850
F 0 "U1" H 2175 975 50  0000 C CNN
F 1 "mini_esp32" H 2175 884 50  0000 C CNN
F 2 "ESP32_mini_KiCad_Library:ESP32_mini" H 2300 950 50  0001 C CNN
F 3 "" H 2300 950 50  0001 C CNN
	1    2150 850 
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1600 2550 1500 2550
Wire Wire Line
	2650 2850 2750 2850
Wire Wire Line
	2650 2950 2750 2950
Wire Wire Line
	2650 3150 2750 3150
Wire Wire Line
	2650 3350 2750 3350
Wire Wire Line
	2650 3250 2750 3250
Wire Wire Line
	1550 850  1550 950 
Wire Wire Line
	1550 950  1600 950 
Wire Wire Line
	1350 850  1350 1050
Wire Wire Line
	1350 1050 1600 1050
Wire Wire Line
	1400 1200 1400 1150
Wire Wire Line
	1400 1150 1600 1150
$Comp
L MicroUSBBreakout:MicroUSBBreakout U2
U 1 1 61B73DCC
P 7200 1400
F 0 "U2" H 7133 1825 50  0000 C CNN
F 1 "MicroUSBBreakout" H 7133 1734 50  0000 C CNN
F 2 "MicroUSBBreakout:MicroUSBBreakout" H 7200 1400 50  0001 C CNN
F 3 "" H 7200 1400 50  0001 C CNN
	1    7200 1400
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0114
U 1 1 61B74BBC
P 7650 1100
F 0 "#PWR0114" H 7650 950 50  0001 C CNN
F 1 "+5V" V 7665 1228 50  0000 L CNN
F 2 "" H 7650 1100 50  0001 C CNN
F 3 "" H 7650 1100 50  0001 C CNN
	1    7650 1100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0115
U 1 1 61B78BA0
P 7650 1650
F 0 "#PWR0115" H 7650 1400 50  0001 C CNN
F 1 "GND" V 7655 1522 50  0000 R CNN
F 2 "" H 7650 1650 50  0001 C CNN
F 3 "" H 7650 1650 50  0001 C CNN
	1    7650 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	7400 1600 7650 1600
Wire Wire Line
	7650 1600 7650 1650
Wire Wire Line
	7650 1100 7650 1200
Wire Wire Line
	7650 1200 7400 1200
$EndSCHEMATC
