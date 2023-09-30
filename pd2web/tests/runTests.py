import os
import sys

TestFolder = os.path.dirname(os.path.realpath(__file__))

# list all folders in the test folder
errors = 0
for root, folders, files in os.walk(TestFolder):
    for folder in folders:
        if folder != ".backup" and folder != "webpatch":
            for root, folders, files in os.walk(os.path.join(TestFolder, folder)):
                for file in files:
                    if file.endswith(".pd") and file == f"{folder}.pd":        
                        testFile = os.path.join(TestFolder, folder, f"{folder}.pd")        
                        returnCode = os.system(f"pd2wasm --patch {testFile}")                
                        if returnCode != 0:
                            errors += 1

if errors > 0:
    print(f"Found {errors} errors.")
    sys.exit(1)
else:
    print("All tests passed.")
    sys.exit(0)






