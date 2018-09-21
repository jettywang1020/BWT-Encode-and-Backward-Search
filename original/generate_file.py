
fread = open('large_file_delimiter.txt', "r")
fwrite = open('10mb_delimiter.txt', "a")

#line = fread.readline()
#cnt = 0
#
#while line:
#	
#	fwrite.write(line)
#	
#	
#	cnt += 1
#	line = fread.readline()
#	
#	
#	if cnt == 900000:
#		print(cnt)
#		break



c = fread.read(10000000)
fwrite.write(c)











fwrite.close()
fread.close()