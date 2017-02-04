import math
import decimal

print("*** Proportioanl Variable Radius Arc Length Calculator ***")
print("")
print("")

def chord_length(start_radius,stop_radius,radians):
    c = math.sqrt((math.pow(start_radius,2))+(math.pow(stop_radius,2))-(2*start_radius*stop_radius*math.cos(radians)))
    return c

def arc_length(initial_radius, final_radius, num_of_steps, num_of_revs):

    length = 0

    for i in range(1,num_of_steps+1):
        calc_length = chord_length(initial_radius+(((final_radius-initial_radius)*(i-1))/num_of_steps),initial_radius+(((final_radius-initial_radius)*i)/num_of_steps),((num_of_revs*2*math.pi)/num_of_steps))
        length += calc_length
        #print(calc_length)
        #print(i)

    return length

IR = input("Input the initial radius (must be > than 0): ")
FR = input("Input the final radius (must be > than 0): ")
NOS = input("Input the number of steps (must be integer number): ")
NOR = input("Number of complete revolutions (can be decimal or integer number): ")

total_arc_length = arc_length(float(IR),float(FR),int(NOS),float(NOR))

print(total_arc_length)
