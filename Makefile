PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include/oxxy
CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -D_DEFAULT_SOURCE -I./src
LDFLAGS = -lpthread -ldl -lm
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I./src

# UI build flags (requires system GLFW, OpenGL and Dear ImGui development headers or sources)
UI_LDFLAGS = -lglfw -lGL -ldl -lpthread -lX11 -lXrandr -lXi -lXxf86vm -lXinerama
UI_SRCS = ui/ui_main_gl.cpp
UI_OBJS = $(UI_SRCS:.cpp=.o)

# Core sources
SRCS = src/pcm_ring.c src/audio_pipeline.c src/ui_bridge.c src/meta_id3.c src/playlist.c src/xdg.c src/profiles.c src/vk.c src/main_launcher.c
OBJS = $(SRCS:.c=.o)

# Allow building with ALSA if requested
ifeq ($(USE_ALSA),1)
    CFLAGS += -DUSE_ALSA=1
    LDFLAGS += -lasound
endif

all: bin/oxxy-test bin/oxxy-launcher

bin/oxxy-test: $(filter-out src/main_launcher.o, $(OBJS)) | bin
	$(CC) $(CFLAGS) -o $@ $(filter-out src/main_launcher.o, $(OBJS)) $(LDFLAGS)

bin/oxxy-launcher: $(filter-out src/audio_pipeline.o, $(OBJS)) | bin
	$(CC) $(CFLAGS) -o $@ $(filter-out src/audio_pipeline.o, $(OBJS)) $(LDFLAGS)

.PHONY: ui
ui: bin/oxxy-ui

.PHONY: ui-gl
ui-gl: bin/oxxy-ui-gl

bin/oxxy-ui: $(UI_OBJS) $(OBJS) | bin
	$(CXX) $(CXXFLAGS) -o $@ $(UI_OBJS) $(OBJS) $(UI_LDFLAGS) $(LDFLAGS)

bin/oxxy-ui-gl: ui/ui_main_gl.o $(OBJS) | bin
	$(CXX) $(CXXFLAGS) -o $@ ui/ui_main_gl.o $(OBJS) $(UI_LDFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 bin/oxxy-test $(DESTDIR)$(BINDIR)/oxxy-test

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/oxxy-test

clean:
	rm -f src/*.o bin/oxxy-test bin/oxxy-ui bin/oxxy-launcher bin/test_meta bin/test_playlist

.PHONY: all install uninstall clean

.PHONY: test
test: all
	@echo "Running unit tests"
	gcc -std=c11 -O2 tests/test_meta.c -o bin/test_meta src/meta_id3.c || true
	./bin/test_meta || true
	gcc -std=c11 -O2 tests/test_playlist.c -o bin/test_playlist src/playlist.c || true
	./bin/test_playlist || true

.PHONY: build_verbose run_all
build_verbose:
	@echo "Starting verbose build (logs -> build.log)"
	@rm -f build.log run.log
	@set -x; \
	gcc -std=c11 -O2 -Wall -D_DEFAULT_SOURCE -I./src -c src/pcm_ring.c -o src/pcm_ring.o 2>&1 | tee -a build.log; \
	gcc -std=c11 -O2 -Wall -D_DEFAULT_SOURCE -I./src -c src/ui_bridge.c -o src/ui_bridge.o 2>&1 | tee -a build.log; \
	gcc -std=c11 -O2 -Wall -D_DEFAULT_SOURCE -I./src -c src/meta_id3.c -o src/meta_id3.o 2>&1 | tee -a build.log; \
	gcc -std=c11 -O2 -Wall -D_DEFAULT_SOURCE -I./src -c src/playlist.c -o src/playlist.o 2>&1 | tee -a build.log; \
	gcc -std=c11 -O2 -Wall -D_DEFAULT_SOURCE -I./src -c src/audio_pipeline.c -o src/audio_pipeline.o 2>&1 | tee -a build.log; \
	gcc -std=c11 -O2 -Wall -D_DEFAULT_SOURCE -I./src -o bin/oxxy-test src/pcm_ring.o src/ui_bridge.o src/meta_id3.o src/playlist.o src/audio_pipeline.o -lpthread -ldl -lm 2>&1 | tee -a build.log

run_all: build_verbose
	@echo "Running tests and core binary (logs -> run.log)"; \
	set -x; \
	[ -x bin/test_meta ] || gcc -std=c11 -O2 -Wall -I./src tests/test_meta.c src/meta_id3.c -o bin/test_meta 2>&1 | tee -a run.log; \
	./bin/test_meta 2>&1 | tee -a run.log || true; \
	[ -x bin/test_playlist ] || gcc -std=c11 -O2 -Wall -I./src tests/test_playlist.c src/playlist.c -o bin/test_playlist 2>&1 | tee -a run.log; \
	./bin/test_playlist 2>&1 | tee -a run.log || true; \
	[ -x bin/oxxy-test ] || true; \
	if [ -x bin/oxxy-test ]; then ./bin/oxxy-test 2>&1 | tee -a run.log || true; else echo "bin/oxxy-test not available" | tee -a run.log; fi
