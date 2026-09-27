CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread

DIRS = 01-deadlock 02-livelock 03-starvation
BINS = $(foreach d,$(DIRS),$(d)/problem $(d)/solution)

all: $(BINS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

run: all
	@for d in $(DIRS); do \
	  for p in problem solution; do \
	    printf "\n===== $$d/$$p =====\n"; ./$$d/$$p; \
	  done; \
	done

clean:
	rm -f $(BINS)

.PHONY: all run clean
