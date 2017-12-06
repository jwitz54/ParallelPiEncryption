import inotify.adapters
import os


dir_to_monitor = '/home/pi/test'

i = inotify.adapters.Inotify()

i.add_watch(dir_to_monitor)
file_path = ""
file_tokens = list()
while 1:
	for event in i.event_gen():
		if event is not None:
			(_, type_names, path, filename) = event
			if(type_names[0]=='IN_CREATE'):
				#user scp information
				if("userfile" in filename):
					#open filename in directory
					#print(path + '/' + filename)
					file_path = str(path + '/' + filename)
					
					fp = open(file_path, 'r')
					print(fp.read())
					#get relevant information from file, 0=ip, 1=user, 2=pass
					#file_tokens = file_string.split(',')
					#user_file.close()
					#remove file
					os.system("rm " + path+'/'+filename)
					#print(file_tokens)
					break
				else:
					
					print("Encrypting file: " + filename)
					print(file_tokens)
					#os.system("./tea_omp")
					print("Removing file: " + filename)
					os.system("rm " + path+'/'+filename)
					#scp encrypted_file user@ip
					os.system("scp " + 'jon_mpi_TEA.c ' + file_tokens[1] + '@' + file_tokens[0]+':/tmp/')
					#write pass
					os.system(file_tokens[2]) 
					break
			
#os.system("python test_inotify.py")

