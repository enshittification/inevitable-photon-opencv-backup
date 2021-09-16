PHP_CONFIG=php-config
PKGC_LIBS=libheif opencv4 exiv2 lcms2 libwebpdemux libwebpmux
CXXFLAGS=-Wall -Wextra -O3 -std=c++17 -fpic -isystem vendor \
		`pkg-config --cflags $(PKGC_LIBS) \
			| sed -E "s/(^| )-I/\1-isystem /g"` \
		`$(PHP_CONFIG) --includes | sed -E "s/(^| )-I/\1-isystem /g"`
LDLIBS=-lphpcpp -lgif \
			 `pkg-config --libs $(PKGC_LIBS)`
LDFLAGS=-shared

OBJECTS=photon-opencv.o tempfile.o
DECODER_HEADERS=decoder.h giflib-decoder.h opencv-decoder.h libwebp-decoder.h \
	libheif-decoder.h

all: photon-opencv.so

photon-opencv.o: vendor/msf_gif.h $(DECODER_HEADERS)

vendor/msf_gif.h: vendor/msf_gif_bgr.patch vendor/msf_gif_rc.h
	cp vendor/msf_gif_rc.h "$@"
	patch "$@" vendor/msf_gif_bgr.patch

photon-opencv.so: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

install: all
	install photon-opencv.so `$(PHP_CONFIG) --extension-dir`

clean:
	rm -f photon-opencv.so $(OBJECTS) vendor/msf_gif.h
