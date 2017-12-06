import sys
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

def wait_for_encryption_result(socket):
	msg = ""
	while(1):
		packet = socket.recv(1)
		msg+=packet
		if(msg=="FINISHED_ENCRYPTION"):
			print(msg)
			break
	return 

#host = "127.0.0.1" #RPi Server
host = 'localhost'
port = 8765
working_dir = 'test_server/'

#arguments are paths to files user wants encrypted
arguments = sys.argv[1:]
print(arguments)
num_files = len(arguments)
file_obj_list = []

for files in arguments:
	file_obj = open(files, "rb")
	file_obj_list.append(file_obj)

#sets up connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))
#sends over number of files to encrypt
#delim_message = "Number of files:" + str(num_files)
s.send(str(num_files)+'\n')

#send files over to be encrypted
for i in range(num_files):
	#preparing message for server telling it the file name and size
	file_name = arguments[i].split("/")[-1]
	file_size = str(os.stat(arguments[i]).st_size)
	message = str(file_name+','+file_size)
	s.send(message+'\n')
	print file_size
	#work_file = open(working_dir + file_name_, "rb")
	print("sending file: %s" % file_name)
	file_size = int(file_size)
	#keep writing file over to server until no more
	while file_size > 0:
		write_buff = file_obj_list[i].read(1024)
		print(write_buff)
		s.send(write_buff)
		file_size -= len(write_buff)


wait_for_encryption_result(s) 
#busy wait, try to find how to throw interrupts in python
#do nothing

s.shutdown(1)
#read file sent over from client
files_received = 0
while files_received < num_files:
	#read_buff = collect_input(client).rstrip('\n').split(',')
	read_buff = collect_input(s).rstrip('\n').split(',')
	print(read_buff)
	file_name = read_buff[0]
	file_size = int(read_buff[1])
	f = open((working_dir + file_name), "wb+")
	while file_size > 0:

		if(file_size<1024):
			encrypted_file = s.recv(file_size)
		else:
			encrypted_file = s.recv(1024)
		#write buffer into file
		f.write(encrypted_file)
		# subtract the actual number of bytes received
		file_size -= len(encrypted_file)
	f.close()
	files_received += 1





#def send(socket, message):
    # In Python 3, must convert message to bytes explicitly.
    # In Python 2, this does not affect the message.
 #   socket.send(message.encode('utf-8'))