import os

TestFolder = os.path.dirname(os.path.realpath(__file__))

# list all folders in the test folder
for folder in os.listdir(TestFolder):
    if folder != os.path.basename(TestFolder):
        testFile = os.path.join(TestFolder, folder, "main.pd")
        os.system(f"pd2wasm --patch {testFile}")







