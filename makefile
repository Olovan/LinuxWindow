All: program 

program:
	gcc LinuxWindow.c -o LinuxWindow -lGL -lGLU -lX11 -lm

clean:
	rm ./*.o
