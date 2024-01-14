all: master writer reader
clean-logs:
	rm log/logfile.txt
	touch log/logfile.txt
clean:
	rm build/*
master:

	gcc master.c -o build/master 

writer:
	gcc writer.c -o build/writer -lncurses

reader:
	gcc reader.c -o build/reader 


