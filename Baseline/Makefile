.PHONY: install, run
CC = gcc
CPP = g++
# include openGL header
INCLUDE = -I /usr/local/include
INCLUDE = -L /usr/local/lib
SOURCES		= main.cpp 
GLAD_SOURCE = glad.cpp
# Corresponding to the need to link libraries
LIBS 	+= -lGL -lGLU -lglut
LIBS    += -lopencv_core -lopencv_highgui -lopencv_aruco -lopencv_imgproc -lopencv_calib3d -lopencv_features2d -lopencv_imgcodecs  
LIBS    += -lopencv_videoio -lopencv_xfeatures2d -ldl -lglfw -lGL -lassimp
# make 
run: install
	./main
install: main.cpp glad.c
	@$(CPP) $(INCLUDE) -o main
