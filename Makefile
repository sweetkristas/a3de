
OPTIMIZE=yes
CCACHE?=ccache
USE_CCACHE?=$(shell which $(CCACHE) 2>&1 > /dev/null && echo yes)
ifneq ($(USE_CCACHE),yes)
CCACHE=
endif

ifeq ($(OPTIMIZE),yes)
BASE_CXXFLAGS += -O2
endif

BASE_CXXFLAGS += -g -fno-inline-functions -fthreadsafe-statics -Wall

SDL2_CONFIG?=sdl2-config
USE_SDL2?=$(shell which $(SDL2_CONFIG) 2>&1 > /dev/null && echo yes)

ifneq ($(USE_SDL2),yes)
$(error SDL2 is not installed on your system.)
endif

INC := -Isrc $(shell pkg-config --cflags sdl2 SDL2_image libpng zlib SDL2_ttf SDL2_mixer)
LIBS := -llua52 -ldl -lboost_regex -lboost_system -lboost_thread\
	$(shell pkg-config --libs sdl2 SDL2_image libpng zlib SDL2_ttf SDL2_mixer)

objects = \
	src/lua1.o

src/%.o : src/%.cpp
	@echo "Building:" $<
	@$(CCACHE) $(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -DIMPLEMENT_SAVE_PNG -c -o $@ $<
	@$(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -DIMPLEMENT_SAVE_PNG -MM $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|src/$*.o:|' < $*.d.tmp > src/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> src/$*.d
	@rm -f $*.d.tmp

a3de: $(objects)
	@echo "Linking : a3de"
	@$(CCACHE) $(CXX) \
		$(BASE_CXXFLAGS) $(LDFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) \
		$(objects) -o a3de \
		$(LIBS) -fthreadsafe-statics

clean:
	rm -f src/*.o src/*.d a3de

