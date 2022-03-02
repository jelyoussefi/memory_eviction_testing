
import matplotlib.pyplot as plt
from scipy.interpolate import splrep, splev
import json

f = open('./output/lowPrio.json')
configLP = json.load(f)
f = open('./output/highPrio.json')
configHP = json.load(f)
f.close()

totalMemGb = float(configLP["totalMemSize"])/(1024*1024*1024)

xLP, yLP, xHP, yHP = [], [], [], []
 

for row in open('./output/lowPrio.dat','r'):
    row = row.split()
    xLP.append(int(row[0]))
    yLP.append(float(row[1]))


for row in open('./output/highPrio.dat','r'):
    row = row.split()
    xHP.append(int(row[0]))
    yHP.append(float(row[1]))


bspl = splrep(xLP,yLP,s=1)
yLP = splev(xLP,bspl)

bspl = splrep(xHP,yHP,s=1)
yHP = splev(xHP,bspl)

x0 = min(xLP + xHP)

splitIdx = -1

for i in range(0, len(xLP)-1)  :
	if ( xLP[i+1] - xLP[i] ) > 10000:
		splitIdx = i;
		break;
		
if splitIdx != -1:
	x1LP = xLP[:splitIdx]
	y1LP = yLP[:splitIdx]
	x2LP = xLP[splitIdx+1:]
	y2LP = yLP[splitIdx+1:]
	xLPLimit = [ x1LP[0], x1LP[-1], x2LP[0], x2LP[-1] ]
	yLPLimit = [ y1LP[0], y1LP[-1], y2LP[0], y2LP[-1] ]
else:
	x1LP = xLP
	y1LP = yLP
	xLPLimit = [ x1LP[0], x1LP[-1] ]
	yLPLimit = [ y1LP[0], y1LP[-1] ]



xHPLimit = [ xHP[0], xHP[-1] ]
yHPLimit = [ yHP[0], yHP[-1] ]

	
x1LP[:] = [float(x - x0)/1000 for x in x1LP]
xHP[:] = [float(x - x0)/1000 for x in xHP]
if splitIdx != -1:
	x2LP[:] = [float(x - x0)/1000 for x in x2LP]
xLPLimit[:] = [float(x - x0)/1000 for x in xLPLimit]
xHPLimit[:] = [float(x - x0)/1000 for x in xHPLimit]


plt.plot(x1LP, y1LP, color='g', label='Low Priority Process ({}%)'.format(int(configLP['memSizeRatio']*100)))
if splitIdx != -1:
	plt.plot(x2LP, y2LP, color='g')
plt.plot(xHP, yHP, color='r', label='High Priority Process ({}%)'.format(int(configHP['memSizeRatio']*100)))
plt.scatter(xLPLimit, yLPLimit, color='g')
plt.scatter(xHPLimit, yHPLimit, color='r')
plt.hlines(totalMemGb, xLPLimit[0], xLPLimit[-1], color='b', linestyles='dashed', label='Total Memory size (GB)')
plt.legend(loc="lower right")
plt.xlabel('Timestamp (s)')
plt.ylabel('Allocated memory size (GB)')
plt.xlim(xmin=0)
plt.ylim(ymin=0)
plt.title("Memory Eviction");
plt.show()



