CC=clang
CXX=clang++
CXXFLAGS= -I. -fPIC
AR=llvm-ar
RM=rm -f
LIB=libimgui.a

SRCS=$(shell printf "%s " *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(LIB): $(OBJS)
	$(AR) cr $@ $(OBJS)

clean:
	$(RM) *.o *.d $(LIB)