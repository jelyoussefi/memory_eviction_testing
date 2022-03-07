
import matplotlib.pyplot as plt
from scipy.interpolate import splrep, splev
import json


def plotMem(plotter, memFile, confFile, color, label, x0=None):
	f = open(confFile)
	config = json.load(f)
	f.close()

	totalMemSizeGb = float(config["totalMemSize"])/(1024*1024*1024)
	memSizeGb = totalMemSizeGb*float(config['memSizeRatio'])
	memRatio = " ({}%) ".format(int(config['memSizeRatio']*100))

	x, y = [], []
	for row in open(memFile):
		row = row.split()
		x.append(int(row[0]))
		y.append(float(row[1]))

	bspl = splrep(x, y, s=2)
	y = splev(x, bspl)

	suspendIdx = None
	for i in range(0, len(x)-1)  :
		if ( x[i+1] - x[i] ) > 1000:
			suspendIdx = i;
			break;
	if x0 == None:
		x0 = x[0];
	x = [float(v-x0)/1000 for v in x]

	resumeIdx = None
	if suspendIdx != None:
		for i in range(suspendIdx+1, len(x)):
			if y[i] >= (memSizeGb):
				resumeIdx = i
				break;

	plotter.plot(x[:suspendIdx], y[:suspendIdx], lw=2, color=color, marker='o', label=label+memRatio, markevery=[0,-1])
	if suspendIdx != None:
		plotter.plot(x[suspendIdx+1:], y[suspendIdx+1:],  lw=2, color=color, marker='o', markevery=[0,-1])
	if resumeIdx != None:
		plt.vlines(x[suspendIdx+1], y[suspendIdx+1], memSizeGb, lw=1, color='k', linestyles='dashed')
		plt.vlines(x[resumeIdx], y[suspendIdx+1], memSizeGb, lw=1, color='k', linestyles='dashed')
		resumeMemSize = y[suspendIdx+1] + (memSizeGb-y[suspendIdx+1] )/2
		plt.plot([x[suspendIdx+1], x[resumeIdx]], [resumeMemSize, resumeMemSize],
			     color='c', marker='.', label='Resume time {} s'.format(format(x[resumeIdx]-x[suspendIdx+1], '.1f')))

	return x0;

def plotPerf(plotter, perfFile, confFile, color, x0, label=None):
	f = open(confFile)
	config = json.load(f)
	f.close()

	totalMemSizeGb = float(config["totalMemSize"])/(1024*1024*1024)
	memSizeGb = totalMemSizeGb*float(config['memSizeRatio'])
	memRatio = " ({}%) ".format(int(config['memSizeRatio']*100))

	x, y = [], []
	for row in open(perfFile):
		row = row.split()
		x.append(int(row[0]))
		y.append(float(row[1]))

	x = [float(v-x0)/1000 for v in x]

	plotter.bar(x, y, width=0.1, color=color, alpha=0.2, label=label)
	plotter.fill([x[0],x[1],x[1],x[0],x[0]],[0, 0,20,20,0], color=color, alpha=0.5)
	plt.text(x[0] + (x[1]-x[0])/2, 10, "Init", color='w', horizontalalignment='center', verticalalignment='center', wrap=False )

		
if __name__ == "__main__":
	fig,axMem = plt.subplots()
	x0 = plotMem(axMem, './output/lowPrio.dat', './output/lowPrio.json', 'g', "LP Allocated Mem.")
	plotMem(axMem, './output/highPrio.dat', './output/highPrio.json', 'r', "HP Allocated Mem.", x0=x0)
	axMem.set_ylabel('Allocated memory size (GB)')
	axMem.set_xlabel('Timestamp (s)')
	axMem.set_ylim(ymin=0)

	axPerf = axMem.twinx()
	plotPerf(axPerf, './output/lowPrioPerf.dat', './output/lowPrio.json', 'g', x0=x0, label="LP Processing time")
	plotPerf(axPerf, './output/highPrioPerf.dat', './output/highPrio.json', 'r', x0=x0, label="HP Processing time")
	#axPerf.set_yscale('log')
	axPerf.set_ylim(ymin=0)
	axPerf.set_ylabel('Processing Time (ms)')

	plt.title("Memory Eviction");
	lines, labels = axMem.get_legend_handles_labels()
	lines2, labels2 = axPerf.get_legend_handles_labels()
	axMem.legend(lines + lines2, labels + labels2, loc=0)
	
	plt.show()



