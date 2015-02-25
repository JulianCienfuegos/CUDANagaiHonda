from numpy import *
import matplotlib.pyplot as plt

x,y = loadtxt('energy.dat', unpack=1, usecols=[0,1])

fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.set_title("Energy in the mesh")    
ax1.set_xlabel('Iteration')
ax1.set_ylabel('Energy')
ax1.set_axis_bgcolor('orange')
ax1.plot(x,y, c='black', label='Energy')
plt.savefig('Images/energy.png')
