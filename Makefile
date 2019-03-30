sim: shell.c pipe.c
	gcc -g -O2 $^ -o $@
	./$@ test1.mem
.PHONY: clean
clean:
	rm -rf *.o *~ sim
