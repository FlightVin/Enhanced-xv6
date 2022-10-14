from pprint import pprint
import queue
import matplotlib.pyplot as plt

lists = [[[0], [0]], [[0], [0]], [[0], [0]], [[0], [0]], [[0], [0]]] 

with open("test.txt") as fobj:
    data_ = fobj.readlines()
    for i in data_:
        new_i = i.strip()
        if (new_i != ""):
            new_i = new_i.split()

            # print(new_i)

            pid_field = new_i[0]
            ticks_field = new_i[1]
            queue_field = new_i[2]

            pid_field = pid_field.strip("((pid: ))")
            ticks_field = ticks_field.strip("((ticks: ))")
            queue_field = queue_field.strip("((queuenum: ))")

            # print(pid_field, ticks_field, queue_field

            pid_field = int(pid_field)
            ticks_field = int(ticks_field)
            queue_field = int(queue_field)


            lists[pid_field - 9][0].append(ticks_field - 1)
            lists[pid_field - 9][1].append(lists[pid_field - 9][1][-1])

            lists[pid_field - 9][0].append(ticks_field)
            lists[pid_field - 9][1].append(queue_field)


for i in range(5):
    plt.plot(lists[i][0], lists[i][1], label = "Process " + str(i))

plt.legend()
plt.savefig("mlfq_plot.png") 
