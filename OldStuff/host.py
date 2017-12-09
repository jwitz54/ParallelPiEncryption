import socket
import os


def collect_input(socket):
	msg = ""
	escape_chars = ""
	while(1):
		packet = socket.recv(1)
		msg+=packet
		escape_chars = msg[-2:] 
		if (escape_chars[-1:] == "\n"): break
	return msg

working_dir = "/tmp/"
#host = "127.0.0.1" #ip of raspberry pi, local host for now
host = '10.148.13.142' #ip of raspberry pi
#host='localhost'
port = 8765


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.settimeout(100)
server.bind((host, port))

server.listen(5)


while True:
	print("Waiting for connection") 
	client, address = server.accept()
	print("Connected with + %s" % str(address) )

	#client will send number of files it wants encrypted
	#delim_msg = str(client.recv(16)).split(":")
	num_files = int(collect_input(client).rstrip('\n'))
	#print(num_files)
	print("number of files: %d" % num_files)


	
	file_names = []
	files_received = 0
	#read files sent over from client to be encrypted
	while files_received < num_files:

		#split the read text into list that has index 0 = file_name and index 1 = file_size
		read_buff = collect_input(client).rstrip('\n').split(',')
		#create list of file names
		file_name = read_buff[0]
		file_names.append(file_name)
		file_size = int(read_buff[1])
		f = open((working_dir + file_name), "wb+")

		print("Receiving: %s Size: %d\n" % (file_name, file_size))
		#keep reading and writing to file until all finished reading everything
		while file_size > 0:

			print("Writing file")
			#check size of file, read # of appropriate bytes if less than 1024, otherwise keep reading bytes
			if file_size<1024:
				work_file = client.recv(file_size)
			else:
				work_file = client.recv(file_size)
			#write buffer into file
			f.write(work_file)
			# subtract the actual number of bytes received
			file_size -= len(work_file)
		f.close()
		files_received+=1

	#perform encryption on file
	os.system("echo 'running encryption/decryption algorithm %s' " % "broooo" )

	client.send("FINISHED_ENCRYPTION")

	#print(collect_input(client))

	#send encrypted files back
	for i in range(num_files):
		file_name = file_names[i]

		#for now there is no encrypted so sending same file back to test
		file_size = str(os.stat(working_dir+file_name).st_size)
		message = str(file_name + ',' + file_size+'\n')
		print("msg to be sent: %s" % message)
		#print file_size
		client.send(message)
		file_to_send = open(working_dir + file_name, "rb")
		print "sending file"
		file_size = int(file_size)
		while file_size > 0:
			write_buff = file_to_send.read(1024)
			client.send(write_buff)
			file_size -= len(write_buff)

	print "sent encrypted files back"
	print("removing files sent that were sent over")
	for files in file_names:
		os.system("rm "+working_dir+files)
		
	client.close()






  #c.close()
