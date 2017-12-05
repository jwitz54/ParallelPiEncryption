import inotify.adapters
import os


dir_to_monitor = '/home/pi/test'

i = inotify.adapters.Inotify()

i.add_watch(dir_to_monitor)


for event in i.event_gen():
	if event is not None:
		(_, type_names, path, filename) = event
		print(type_names)
		if(type_names[0]=='IN_CREATE'):
			print("Encrypting file: " + filename)
			os.system("./tea_omp")
			print("Removing file: " + filename)
			os.system("rm " + path+'/'+filename)
			break
		
os.system("python test_inotify.py")

