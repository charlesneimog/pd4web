TARGET = webpatch/libpd.js
LIBPD_DIR = libpd

SRC_FILES = webpatch/main.c
EXTERNAL_FILES = $(wildcard webpatch/externals/*.c)

LIBS_DATA_FILES = $(wildcard webpatch/libs/*.pd)
LIBS_DATA_WITH_FLAGS = $(foreach file,$(LIBS_DATA_FILES),--preload-file $(file))

CFLAGS = -I webpatch/extra/ -I $(LIBPD_DIR)/pure-data/src -I $(LIBPD_DIR)/libpd_wrapper 
CFLAGS += -L $(LIBPD_DIR)/build/libs -lpd

LDFLAGS = -O3  
LDFLAGS += -s AUDIO_WORKLET=1 -s WASM_WORKERS=1 -s ALLOW_MEMORY_GROWTH=1 \

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
		$(LIBS_DATA_WITH_FLAGS) \
		--preload-file webpatch/index.pd \
		$(EXTRA_DATA_FILES) \
		$(SRC_FILES) $(EXTERNAL_FILES) -o $(TARGET) 
				 




