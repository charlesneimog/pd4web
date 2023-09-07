# USER OPTIONS
NOTCLEAR = 0

ifeq ($(NOTCLEAR), 1)
	TMP_FILES = --clearTmpFiles False
endif

target: patch makewebpatch 

# check if user run this as make PATCH=patch_name, if just make, then give error
patch:
ifndef PATCH
	$(error PATCH is undefined, please run as make PATCH=patch_name)
else

endif
	
makewebpatch:
	python pd2wasm/PdWebCompiler.py --patch $(PATCH) --html $(HTML) $(TMP_FILES)






