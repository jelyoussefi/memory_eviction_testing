import matplotlib.pyplot as plt

xLP, yLP, xHP, yHP = [], [], [], []
 

for row in open('./output/lowPrio.dat','r'):
    row = row.split()
    xLP.append(int(row[0]))
    yLP.append(float(row[1]))


for row in open('./output/highPrio.dat','r'):
    row = row.split()
    xHP.append(int(row[0]))
    yHP.append(float(row[1]))


x0 = min(xLP + xHP)

xLP[:] = [(x - x0) for x in xLP]
xHP[:] = [(x - x0) for x in xHP]
print(xLP)
print(xHP)
plt.plot(xLP, yLP, color='g', label='LP')
plt.plot(xHP, yHP, color='r', label='HP')

plt.legend()

plt.show()
