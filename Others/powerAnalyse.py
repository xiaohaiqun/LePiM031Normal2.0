from matplotlib import pyplot as plt
x=[]
y=[]
with open('powerRecord.txt','r') as f:
    for i in range(823):
        line=f.readline().split('  ')
        #print(line)
        x.append(1000-int(line[0]))
        y.append(float(line[1]))
    #print(x)
    #print(y)
z=list(reversed(x))
plt.plot(y, x, '.', label="powerline")
plt.show()