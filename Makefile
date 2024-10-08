# Makefile for the smash program
CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -Werror -pthread -pedantic-errors -DNDEBUG
CXXLINK = $(CXX)
OBJS = ATM.o bank.o account.o
RM = rm -f
# Creating the  executable
Bank: $(OBJS)
	$(CXXLINK) $(CXXFLAGS) $(OBJS) -o Bank
# Creating the object files
ATM.o: ATM.cpp bank.hpp account.hpp
bank.o: bank.cpp bank.hpp account.hpp
account.o: account.cpp account.hpp
# Cleaning old files before new make
clean:
	$(RM) *.o Bank
