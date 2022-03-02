
import matplotlib.pyplot as plt
from scipy.interpolate import splrep, splev
import json

f = open('./output/lowPrio.json')
configLP = json.load(f)
f = open('./output/highPrio.json')
configHP = json.load(f)
f.close()

totalMemSizeGb = float(configLP["totalMemSize"])/(1024*1024*1024)
lpMemSizeGb = totalMemSizeGb/float(configLP['memSizeRatio'])

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
		
x1LP, y1LP, x2LP, y2LP = [], [], [], []

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
xResume, yResume = [], [] 
if len(x2LP) > 0:
	x2LP[:] = [float(x - x0)/1000 for x in x2LP]
	xResume.append(x2LP[0]);
	yResume.append(lpMemSizeGb/2);
	for i in range(len(y2LP)):
		if y2LP[i] >= lpMemSizeGb:
			xResume.append(x2LP[i]);
			yResume.append(lpMemSizeGb/2);
			break;
	
xLPLimit[:] = [float(x - x0)/1000 for x in xLPLimit]
xHPLimit[:] = [float(x - x0)/1000 for x in xHPLimit]

lw=2.5

plt.plot(x1LP, y1LP, lw=lw, color='g', label='Low Priority Process ({}%)'.format(int(configLP['memSizeRatio']*100)))
if len(x2LP) > 0:
	plt.plot(x2LP, y2LP, lw=lw, color='g')
plt.plot(xHP, yHP, lw=lw, color='r', label='High Priority Process ({}%)'.format(int(configHP['memSizeRatio']*100)))
plt.scatter(xLPLimit, yLPLimit, color='g')
plt.scatter(xHPLimit, yHPLimit, color='r')
plt.hlines(totalMemSizeGb, xLPLimit[0], xLPLimit[-1], color='b', linestyles='dashed', label='Total Memory size (GB)')
if len(xResume) > 0:
	plt.vlines(xResume[0], 0, totalMemSizeGb, lw=1, color='k', linestyles='dashed')
	plt.vlines(xResume[1], 0, totalMemSizeGb, lw=1, color='k', linestyles='dashed')
	plt.plot(xResume, yResume, color='c', marker='o', label='Resume time {} ms'.format(format(xResume[1]-xResume[0], '.1f')))

plt.legend(loc="lower right")
plt.xlabel('Timestamp (s)')
plt.ylabel('Allocated memory size (GB)')
plt.xlim(xmin=0)
#plt.ylim(ymin=0)
plt.title("Memory Eviction");
plt.show()



