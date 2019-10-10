CC := g++ -g
CFLAGS := -Wfatal-errors -std=c++17
LFLAGS := -ldl -lMLogger  -lpthread

INLCUDEDIR := include
LIBDIR := lib

SRCDIR := src
OBJDIR := obj
BUILDDIR := build

OUT := build/build.x

SRCFILES := $(shell find ./$(SRCDIR) -name "*.cpp")
OBJFILES := $(subst src,obj,$(subst .cpp,.o,$(SRCFILES)))


all: $(OUT)

$(OUT): $(OBJFILES)
	$(CC) $(OBJFILES) -o $@ -I$(INLCUDEDIR) -L$(LIBDIR) $(LFLAGS) 


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@ 

clean : 
	rm -Rf $(OBJFILES) $(OUT)