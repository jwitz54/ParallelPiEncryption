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
host='localhost'
port = 8765


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#server.settimeout(100)
server.bind((host, port))

server.listen(5)


while True:
	print("Waiting for connection") 
	client, address = server.accept()
	print("Connected with + %s" % str(address) )

	#client will send number of files it wants encrypted
	#delim_msg = str(client.recv(16)).split(":")
	first_msg = collect_input(client).rstrip('\n')
	server_mode = first_msg.split(',')[0]
	num_rounds = first_msg.split(',')[1]
	input_key = first_msg.split(',')[2]
	file_name = first_msg.split(',')[3].split("/")[-1]
	#print(num_files)
	print("mode: %s\tnum_rounds: %s\tinput_key: %s\tfile name: %s\n" % (server_mode, num_rounds, input_key, file_name))


	

	#read files sent over from client to be encrypted


	#split the read text into list that has index 0 = file_name and index 1 = file_size
	file_size = int(collect_input(client).rstrip('\n'))
	#create list of file names
	#file_name = read_buff[0]
	#file_names.append(file_name)
	#file_size = int(read_buff[1])
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


	#perform encryption on file
	if(int(server_mode)==0):
		print("Running encryption for %s rounds with key: %s\n" % (num_rounds, input_key))
		#os.system("echo 'running encryption/decryption algorithm %s' " % "broooo" )
	#perform decryption on file
	elif(int(server_mode)==1):
		print("Running decryption for %s rounds with key: %s\n" % (num_rounds, input_key))
	client.send("FINISHED_PROCESSING")

	#print(collect_input(client))

	#send processed files back (NEED TO CHANGE THIS DEPENDING ON WHAT IS OUTPUT FROM C CODE)
	processed_file_name = file_name

	#for now there is no encrypted so sending same file back to test
	processed_file_size = str(os.stat(working_dir + processed_file_name).st_size)
	message = processed_file_size + '\n'
	client.send(message)
	file_to_send = open(working_dir + file_name, "rb")
	processed_file_size = int(processed_file_size)
	print "Sending proccessed file back\n"
	while processed_file_size > 0:
		write_buff = file_to_send.read(1024)
		client.send(write_buff)
		processed_file_size -= len(write_buff)

	print "Sent Processed Files Back"
	print("Removing Files Sent From Client")
	os.system("rm " + working_dir+file_name)


	client.close()






  #c.close()