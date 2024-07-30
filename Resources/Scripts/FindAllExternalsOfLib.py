import re
import os
import sys


class PdObjectsInSource:
    def __init__(self):
        self.objPerPatch = 10
        self.patch = []
        self.objsFounded = []
        self.libName = sys.argv[1]
        self.libDir = sys.argv[2]
        self.thisFolder = os.path.dirname(os.path.abspath(__file__))
        for root, _, files in os.walk(self.libDir):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    completePath = os.path.join(root, file)
                    self.regexSearch(completePath)
        self.writePatches()

    def regexSearch(self, file):
        with open(file, "r", encoding="utf-8") as c_file:
            file_contents = c_file.read()
            pattern = r'class_new\s*\(\s*gensym\s*\(\s*\"([^"]*)\"\s*\)'
            matches = re.finditer(pattern, file_contents)
            for match in matches:
                objectName = match.group(1)
                self.objsFounded.append(objectName)

    def writePatches(self):
        objCount = 0
        objPosition = 0
        lastWritten = 0
        print("Total of Objects: ", len(self.objsFounded))
        if not os.path.exists(os.path.join(self.thisFolder, self.libName)):
            os.makedirs(os.path.join(self.thisFolder, self.libName))
        for objName in self.objsFounded:
            if objName[0] != "_":
                objCount += 1
                objPosition += 1
                if objCount == 1:
                    self.patch.append("#N canvas 0 0 450 300 10;")
                objPatchPosition = objPosition * 40
                self.patch.append(f"#X obj 20 {objPatchPosition} {self.libName}/{objName};")
                if objCount % self.objPerPatch == 0:
                    obj = objCount - self.objPerPatch
                    patchPath = os.path.join(self.thisFolder, self.libName, f"{self.libName}-{obj}-{objCount - 1}.pd")
                    with open(patchPath, "w") as f:
                        for line in self.patch:
                            f.write(line + "\n")
                    self.patch = []
                    objPosition = 0
                    lastWritten = obj
                    self.patch.append("#N canvas 0 0 450 300 10;")
        if len(self.patch) > 1:
            obj = lastWritten + self.objPerPatch
            patchPath = os.path.join(self.thisFolder, self.libName, f"{self.libName}-{obj}-{objCount - 1}.pd")
            with open(patchPath, "w") as f:
                for line in self.patch:
                    f.write(line + "\n")
            self.patch = []
            objPosition = 0


if __name__ == "__main__":
    PdObjectsInSource()
