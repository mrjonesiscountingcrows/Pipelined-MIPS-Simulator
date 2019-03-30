sim: shell.c pipe.c
	gcc -g -O2 $^ -o $@
	./$@ jumptest.mem
.PHONY: clean
clean:
	rm -rf *.o *~ sim
