import inotify.adapters
import os


dir_to_monitor = '/home/pi/test'

i = inotify.adapters.Inotify()

i.add_watch(dir_to_monitor)


for event in i.event_gen():
	if event is not None:
		(_, type_names, path, filename) = event
		if(type_names[0]=='IN_CREATE'):
			os.system("./tea_omp")
			break
		
os.system("python test_inotify.py")

