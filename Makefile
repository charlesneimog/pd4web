TARGET = webpatch/libpd.js
LIBPD_DIR = libpd

SRC_FILES = webpatch/main.c
EXTERNAL_FILES = $(wildcard webpatch/externals/*.c)
EXTERNAL_FILES += $(wildcard webpatch/externals/*.cpp)

CFLAGS = -I webpatch/extra/ -I $(LIBPD_DIR)/pure-data/src -I $(LIBPD_DIR)/libpd_wrapper 
CFLAGS += -L $(LIBPD_DIR)/build/libs -lpd

LDFLAGS = -O3  
LDFLAGS += -s AUDIO_WORKLET=1 -s WASM_WORKERS=1 -s WASM=1 -s USE_PTHREADS=1 \

# USER OPTIONS
NOTCLEAR = 0

ifeq ($(NOTCLEAR), 1)
	TMP_FILES = --clearTmpFiles False
endif

target: patch makewebpatch emcc

# check if user run this as make PATCH=patch_name, if just make, then give error
patch:
ifndef PATCH
	$(error PATCH is undefined, please run as make PATCH=patch_name)
endif

makewebpatch:
	python resources/webpd.py --patch $(PATCH) $(TMP_FILES)

emcc:
	emcc \
		$(CFLAGS) \
		$(LDFLAGS) \
		--preload-file webpatch/data \
		$(SRC_FILES) $(EXTERNAL_FILES) -o $(TARGET) 
				 




