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

def wait_for_server_result(socket):
	msg = ""
	while(1):
		packet = socket.recv(1)
		msg+=packet
		if(msg=="FINISHED_PROCESSING"):
			print(msg)
			break
	return 


host = 'localhost'
#host = '10.148.13.142'
#host = '10.33.131.208'
port = 8765
working_dir = 'test_server/'

#---------DESCRIPTION OF INPUTS------------------#
#server mode is 0/1; 0=encryption and 1=decryption
#input key must be <=16 characters and end with an !
#number of rounds you want to encrypt/decrypt (MUST DECRYPT THE SAME # OF ROUNDS YOU ENCRYPTED)
#file path to file you wish to encrypt/decrypt



#arguments are encrypt/decrypt(0/1), number of rounds, key (must end with !), and file paths to files 
#user wants encrypted
arguments = sys.argv[1:]

server_mode = arguments[0]
num_rounds = arguments[1]
key = arguments[2]
file_path = arguments[3]
file_name = file_path.split("/")[-1]

#num_files = len(arguments)
#file_obj_list = []

#for files in arguments:
#	file_obj = open(files, "rb")
#	file_obj_list.append(file_obj)

#sets up connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))
print("Established Connection With Server\n")
#sends over number of files to encrypt
#delim_message = "Number of files:" + str(num_files)

print("Sending encryption/decryption parameters\n")
msg_to_send = str(server_mode) + ',' + str(num_rounds) + ',' + str(key) + ',' + str(file_path) + '\n'
s.send(msg_to_send)

#send file over to be encrypted
#preparing message for server telling it the file size
#file_name = arguments[i].split("/")[-1]
file_size = str(os.stat(file_path).st_size)
message = str(file_size)
s.send(message+'\n')
#work_file = open(working_dir + file_name_, "rb")
print("sending file: %s" % file_path.split("/")[-1])
file_size = int(file_size)
file_obj = open(file_path, "rb")
#keep writing file over to server until no more
while file_size > 0:
	write_buff = file_obj.read(1024)
	s.send(write_buff)
	file_size -= len(write_buff)


wait_for_server_result(s) 

#shutdown client socket because done sending, waiting to receive encrypted/decrypted file back
s.shutdown(1)
#read file sent over from client
processed_file_size = int(collect_input(s).rstrip('\n'))
processed_file_name = working_dir+file_name
if int(server_mode)==0:
	print("Receiving Encrypted File\n")
	insert_ind = processed_file_name.find(".")
	new_processed_file_name = processed_file_name[:insert_ind] + '_enc' + processed_file_name[insert_ind:]
	#processed_file_name += '_encrypted'
elif int(server_mode)==1:
	print("Receiving Decrypted File\n")
	insert_ind = processed_file_name.find(".")
	new_processed_file_name = processed_file_name[:insert_ind] + '_enc' + processed_file_name[insert_ind:]
	#processed_file_name += '_decrypted'

f = open((processed_file_name), "wb+")
while processed_file_size > 0:
	if(processed_file_size<1024):
		encrypted_file = s.recv(processed_file_size)
	else:
		encrypted_file = s.recv(1024)
	#write buffer into file
	f.write(encrypted_file)
	# subtract the actual number of bytes received
	processed_file_size -= len(encrypted_file)
print("Received Processed File\n")
print("Terminating Connection With Server\n")
f.close()
#s.close()






#def send(socket, message):
    # In Python 3, must convert message to bytes explicitly.
    # In Python 2, this does not affect the message.
 #   socket.send(message.encode('utf-8'))