all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
#	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw
	sudo g++ `pkg-config --cflags glfw3` -o sample2D Sample_GL3_2D.cpp glad.c `pkg-config --static --libs glfw3`
clean:
	rm sample2D sample3D
