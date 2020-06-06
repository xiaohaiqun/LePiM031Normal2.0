#encoding=utf-8
import time
import smbus
import RPi.GPIO as GPIO
import os
GPIO.setmode(GPIO.BOARD)
GPIO.setup(22,GPIO.IN)
#GPIO.setup(37,GPIO.OUT)
bus =smbus.SMBus(1)
#GPIO.output(37,GPIO.HIGH)
#setting
adress=0x15
###########################
def Byte16ToInt(byte16):
    if((byte16&0x8000)==0):
        r= byte16
    else:
        byte16=byte16^0xffff
        byte16=byte16+1
        r= -byte16
    return r

Button={
        
        'btn1Down':       0x81,
        'btn1long':       0x91,
        'btn1shortandUp': 0x11,
        'btn1Up'  :       0x01,
         
        'btn2Down':       0x82,
        'btn2long':       0x92,
        'btn2shortandUp': 0x12,
        'btn2Up'  :       0x02,

        'btn3Down':       0x83,
        'btn3long':       0x93,
        'btn3shortandUp': 0x13,
        'btn3Up'  :       0x03,

        'btn4Down':       0x84,
        'btn4long':       0x94,
        'btn4shortandUp': 0x14,
        'btn4Up'  :       0x04,

        'btn5Down':       0x85,
        'btn5long':       0x95,
        'btn5shortandUp': 0x15,
        'btn5Up'  :       0x05,

        'btn6Down':       0x86,
        'btn6long':       0x96,
        'btn6shortandUp': 0x16,
        'btn6Up'  :       0x06,

        'btn7Down':       0x87,
        'btn7long':       0x97,
        'btn7shortandUp': 0x17,
        'btn7Up'  :       0x07,

        'btn8Down':       0x88,
        'btn8long':       0x98,
        'btn8shortandUp': 0x18,
        'btn8Up'  :       0x08,
        
        'btn9Down':       0x89,
        'btn9long':       0x99,
        'btn9shortandUp': 0x19,
        'btn9Up'  :       0x09,
        'shutDownRequest':0xA9,
        'PowerInfoChanged':0x55
        }
ReadNineSensor  =0x83
ReadButtonState =0x82

def readpower():
    power=bus.read_i2c_block_data(adress,0x8A,4)
    #print(power[0])
    '''
    if(power[0]==0x1F):
        n_power=100
    elif(power[0]==0x0F):
        n_power=75
    elif(power[0]==0x07):
        n_power=50
    elif(power[0]==0x03):
        n_power=25
    elif(power[0]==0x01):
        n_power=0
    else:
        n_power=0
    '''
    print('\n------------------------------------------------------\n')
    print("剩余电量: "+str(power[0]))
    if((power[1]&0xF0)):
        print("电池正在充电\n")
    else:
        print("电池放电ing\n")
    bat_power_ocv=(power[3]<<8|power[2])
    print("电池开路电压=:"+str(bat_power_ocv))
    print('--------------------------------------------------------\n')
    vout12A=bus.read_i2c_block_data(adress,0x8B,4)
    vout1A=(vout12A[1]<<8|vout12A[0])*0.6394
    vout2A=(vout12A[3]<<8|vout12A[2])*0.6394
    print("vout1A:"+str(vout1A)+"mA     vout2A:"+str(vout2A)+"mA")
   
    BAT=bus.read_i2c_block_data(adress,0x8C,4)
    BAT_V=(vout12A[1]<<8|vout12A[0])*0.26855/1000+2.6
    BAT_A=(vout12A[3]<<8|vout12A[2])*1.27883/1000
    print("电池电压:"+str(BAT_V)+"V     电池电流:"+str(BAT_A)+"A")
    print('---------------------------------------------------------\n')
def readsensor():
    print('---------------------------------------------------------\n')
    acc=bus.read_i2c_block_data(adress,0x83,6)
    acc_x=acc[1]<<8|acc[0]
    acc_x=Byte16ToInt(acc_x)
    acc_y=acc[3]<<8|acc[2]
    acc_y=Byte16ToInt(acc_y)
    acc_z=acc[5]<<8|acc[4]
    acc_z=Byte16ToInt(acc_z)
    print("Acc_X=: "+str(acc_x)+"    Acc_Y=: "+str(acc_y)+"   Acc_Z: "+str(acc_z))
    acc=bus.read_i2c_block_data(adress,0x84,6)
    acc_x=acc[1]<<8|acc[0]
    acc_x=Byte16ToInt(acc_x)
    acc_y=acc[3]<<8|acc[2]
    acc_y=Byte16ToInt(acc_y)
    acc_z=acc[5]<<8|acc[4]
    acc_z=Byte16ToInt(acc_z)
    print("Gyro_X=: "+str(acc_x)+"    Gyro_Y=: "+str(acc_y)+"   Gyro_Z: "+str(acc_z))
    
    acc=bus.read_i2c_block_data(adress,0x85,6)
    acc_x=acc[1]<<8|acc[0]
    acc_x=Byte16ToInt(acc_x)
    acc_y=acc[3]<<8|acc[2]
    acc_y=Byte16ToInt(acc_y)
    acc_z=acc[5]<<8|acc[4]
    acc_z=Byte16ToInt(acc_z)
    print("Magn_X=: "+str(acc_x)+"    Magn_Y=: "+str(acc_y)+"   Magn_Z: "+str(acc_z))
    print("----------------------------------------------------------\n")
flag=0
I2C1InUseFlag=0

def ButtonHandler(channel):
    global I2C1InUseFlag
    if(I2C1InUseFlag==0):
        I2C1InUseFlag=1
        NowButton=bus.read_byte_data(adress,ReadButtonState)
        I2C1InUseFlag=0
    else:
        NowButton=0
    #print(NowButton)
    for b,v in Button.items():
        if(NowButton==v):
            print(b)
        if(NowButton&0xF0==0x50):
            print("---------------------------------------------------")
            print("-------Power state changed-------------------------")
            readpower()
        '''
	if(NowButton==0x99):
            print("DO U want to shutdown?(Y/N):")
            select=raw_input()
            if(select=="Y"):
		os.system("sudo halt")
                GPIO.output(37,GPIO.LOW)
	'''
    pass
GPIO.add_event_detect(22,GPIO.BOTH,callback=ButtonHandler,bouncetime=20)

bus.write_byte_data(adress,0x46,0x47)

while(1):
    #os.system("sudo clear")
    bus.write_byte_data(adress,0x46,0xff)
    try:
        if(I2C1InUseFlag==0):
            I2C1InUseFlag=1
            readsensor()
            readpower()
            #nineBtn=bus.read_i2c_block_data(adress,0x87,9)
            #print(nineBtn)            
            I2C1InUseFlag=0
    except:
        pass
    #bus.write_byte_data(adress,0x41,0x50)
    #print(bus.read_byte_data(adress,0x8D))    
    #time.sleep(500)
    #bus.write_byte_data(adress,0x41,0x51)
    time.sleep(1)
    #os.system("sudo clear")
