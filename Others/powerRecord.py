#encoding=utf-8
import time
import smbus
import RPi.GPIO as GPIO
import os
GPIO.setmode(GPIO.BOARD)
GPIO.setup(22,GPIO.IN)
GPIO.setup(37,GPIO.OUT)
bus =smbus.SMBus(1)
GPIO.output(37,GPIO.HIGH)
#setting
adress=0x15
###########################
Button={
        'btn1down':0x81,
        'btn1up'  :0x01,
        'btn2down':0x82,
        'btn2up'  :0x02,
        'btn3down':0x83,
        'btn3up'  :0x03,
        'btn4down':0x84,
        'btn4up'  :0x04,
        'btn5down':0x85,
        'btn5up  ':0x05,
        'btn6down':0x86,
        'btn6up'  :0x06,
        'btn7down':0x87,
        'btn7up'  :0x07,
        'btn8down':0x88,
        'btn8up'  :0x08,
        'btn9down':0x89,
        'btn9up'  :0x09,
        'shutDownRequest':0x99,
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
    print('\n------------------------------------------------------\n')
    print("剩余电量: "+str(n_power))
    if((power[1]&0xF0)):
        print("电池正在充电\n")
    else:
        print("电池放电ing\n")
    '''
    bat_power_ocv=(power[3]<<8|power[2])*0.26855/1000+2.6
    return bat_power_ocv
    '''
    print("电池开路电压=:"+str(bat_power_ocv)+"v")
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
    '''
def readsensor():
    print('---------------------------------------------------------\n')
    acc=bus.read_i2c_block_data(adress,0x83,6)
    acc_x=acc[1]<<8|acc[0]
    acc_y=acc[3]<<8|acc[2]
    acc_z=acc[5]<<8|acc[4]
    print("Acc_X=: "+str(acc_x)+"    Acc_Y=: "+str(acc_y)+"   Acc_Z: "+str(acc_z))
    acc=bus.read_i2c_block_data(adress,0x84,6)
    acc_x=acc[1]<<8|acc[0]
    acc_y=acc[3]<<8|acc[2]
    acc_z=acc[5]<<8|acc[4]
    print("Gyro_X=: "+str(acc_x)+"    Gyro_Y=: "+str(acc_y)+"   Gyro_Z: "+str(acc_z))
    
    acc=bus.read_i2c_block_data(adress,0x85,6)
    acc_x=acc[1]<<8|acc[0]
    acc_y=acc[3]<<8|acc[2]
    acc_z=acc[5]<<8|acc[4]
    print("Magn_X=: "+str(acc_x)+"    Magn_Y=: "+str(acc_y)+"   Magn_Z: "+str(acc_z))
    print("----------------------------------------------------------\n")
flag=0

def ButtonHandler(channel):
    NowButton=bus.read_byte_data(adress,ReadButtonState)
    print(NowButton)
    for b,v in Button.items():
        if(NowButton==v):
            print(b)
        if(NowButton==0x55):
            print("---------------------------------------------------")
            print("-------Power state changed-------------------------")
            #readpower()
        if(NowButton==0x99):
            print("DO U want to shutdown?(Y/N):")
            select=raw_input()
            if(select=="Y"):
		os.system("sudo halt")
                GPIO.output(37,GPIO.LOW)
    pass
GPIO.add_event_detect(22,GPIO.BOTH,callback=ButtonHandler,bouncetime=20)

bus.write_byte_data(adress,0x46,0x47)
'''
while(1):
    os.system("sudo clear")
    try:
        readsensor()
        readpower()
    except Except:
        pass
    #bus.write_byte_data(adress,0x41,0x50)
    #print(bus.read_byte_data(adress,0x8D))    
    time.sleep(1)
    #bus.write_byte_data(adress,0x41,0x51)
    time.sleep(1)
    os.system("sudo clear")
'''
with open('powerRecord.txt','w') as f:
    t=0
    while(1):
        ocv=readpower()
	'''
        f.write(t)
        f.write(' ')
        f.write(ocv)
        f.write('\n')
        '''
	f.write('%d  %f  \n'%(t,ocv))
	t=t+1
	print('.')
	time.sleep(30)
