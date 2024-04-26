import math
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


log = "../data/DataLogs/Spiral.csv" 

limitFrames = False
countAfterBegin = True

fig = plt.figure()

xt = [] 
yt = [] 
zt = []
xr = [] 
yr = [] 
zr = []
estTime = []
poseValid = []
frameNumber = []
numberGoodEst = 0
numberFrames = 0

solutionBegin = False

def set_axes_equal(ax):
    '''Make axes of 3D plot have equal scale so that spheres appear as spheres,
    cubes as cubes, etc..  This is one possible solution to Matplotlib's
    ax.set_aspect('equal') and ax.axis('equal') not working for 3D.

    Input
    ax: a matplotlib axis, e.g., as output from plt.gca().
    '''

    x_limits = ax.get_xlim3d()
    y_limits = ax.get_ylim3d()
    z_limits = ax.get_zlim3d()

    x_range = abs(x_limits[1] - x_limits[0])
    x_middle = np.mean(x_limits)
    y_range = abs(y_limits[1] - y_limits[0])
    y_middle = np.mean(y_limits)
    z_range = abs(z_limits[1] - z_limits[0])
    z_middle = np.mean(z_limits)

    # The plot bounding box is a sphere in the sense of the infinity
    # norm, hence I call half the max range the plot radius.
    plot_radius = 0.5*max([x_range, y_range, z_range])

    ax.set_xlim3d([x_middle - plot_radius, x_middle + plot_radius])
    ax.set_ylim3d([y_middle - plot_radius, y_middle + plot_radius])
    ax.set_zlim3d([z_middle - plot_radius, z_middle + plot_radius])




with open(log, "r") as file:

    lines = file.read()
    lines = lines.split("\n")

    first = True

    for line in lines:

        if first:
            first = False
            continue

        items = line.split(",")

        if (len(items) == 9):

            if numberFrames >= 500 and limitFrames: break

            poseValidBuf = False

            if items[6] == "1":
                poseValidBuf = True
                solutionBegin = True

            if not countAfterBegin: numberFrames += 1
            elif solutionBegin: numberFrames += 1

            if not poseValidBuf: continue

            xr.append(float(items[0]))
            yr.append(float(items[1]))
            zr.append(float(items[2]))
            xt.append(float(items[3]))
            yt.append(float(items[4]))
            zt.append(float(items[5]))
            poseValid.append(poseValidBuf)
            frameNumber.append(int(items[7]))
            estTime.append(int(items[8]))
            if not countAfterBegin: numberGoodEst += 1
            elif solutionBegin: numberGoodEst += 1


xt = np.array(xt)/10
yt = np.array(yt)/10
zt = np.array(zt)/10
xr = np.array(xr)
yr = np.array(yr)
zr = np.array(zr)

print("Number est:", numberGoodEst, "frames:", numberFrames, "per:", numberGoodEst/numberFrames*100, "%")

#threeDe = plt.subplot2grid((3,3), (0,0), projection='3d', rowspan=3)# fig.add_subplot(3, 2, 1, projection='3d')
threeDe = plt.subplot(projection='3d')

threeDe.plot(xt, yt, zt, linestyle="", marker=".", markersize=2, color="red")
threeDe.set_xlabel('X-Axis')
threeDe.set_ylabel('Y-Axis')
threeDe.set_zlabel('Z-Axis')
#axs.invert_yaxis()
set_axes_equal(threeDe)

"""
print("xt = ", np.array(xt).mean(), " +- ", np.array(xt).std()/math.sqrt(len(xt)), "cm")

plotXt = plt.subplot2grid((3,3), (0,1))
plotXt.plot(frameNumber, xt)
plotXt.set(ylabel='X-Axis [cm]')

print("yt = ", np.array(yt).mean(), " +- ", np.array(yt).std()/math.sqrt(len(yt)), "cm")

plotYt = plt.subplot2grid((3,3), (1,1))
plotYt.plot(frameNumber, yt)
plotYt.set(ylabel='Y-Axis [cm]')

print("zt = ", np.array(zt).mean(), " +- ", np.array(zt).std()/math.sqrt(len(zt)), "cm")

plotZt = plt.subplot2grid((3,3), (2,1))
plotZt.plot(frameNumber, zt)
plotZt.set(ylabel='Z-Axis [cm]', xlabel='Frame number')

plotXt.set_xticklabels([])
plotYt.set_xticklabels([])

print("xr = ", np.array(xr).mean()*180/math.pi, " +- ", np.array(xr).std()/math.sqrt(len(xr))*180/math.pi, "deg")

plotXr = plt.subplot2grid((3,3), (0,2))
plotXr.plot(frameNumber, xr*180/math.pi)
plotXr.set(ylabel='X-Axis [deg]')

print("yr = ", np.array(yr).mean()*180/math.pi, " +- ", np.array(yr).std()/math.sqrt(len(yr))*180/math.pi, "deg")

plotYr = plt.subplot2grid((3,3), (1,2))
plotYr.plot(frameNumber, yr*180/math.pi)
plotYr.set(ylabel='Y-Axis [deg]')

print("zr = ", np.array(zr).mean()*180/math.pi, " +- ", np.array(zr).std()/math.sqrt(len(zr))*180/math.pi, "deg")

plotZr = plt.subplot2grid((3,3), (2,2))
plotZr.plot(frameNumber, zr*180/math.pi)
plotZr.set(ylabel='Z-Axis [deg]')


plt.subplots_adjust(wspace=0.5, hspace=0.2)"""

plt.show()