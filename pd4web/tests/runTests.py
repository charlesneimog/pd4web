import os
import sys
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import threading 
import platform
import time

notFoundObjs = 0
sharedArrayBuffer = 0
compilationReady = False
compilationComplete = 0
browserReady = False



def myprint(str, color=None):
    if color is None:
        print("    " + str)
        return
    if color == 'red':
        print("\033[91m" + "    " + str + "\033[0m")
    elif color == 'green':
        print("\033[92m" + "    " + str + "\033[0m")
    elif color == 'yellow':
        print("\033[93m" + "    " + str + "\033[0m")
    elif color == 'blue':
        print("\033[94m" + "    " + str + "\033[0m")
    elif color == 'magenta':
        print("\033[95m" + "    " + str + "\033[0m")
    elif color == 'cyan':
        print("\033[96m" + "    " + str + "\033[0m")
    else:
        print("    " + str)


def compile(testFile):
    global compilationReady
    global compilationComplete
    compilationReady = False
    compilationComplete = os.system(f"pd4web --patch {testFile} --server-port 8080 --no_browser")
    compilationReady = True


def testinBrowser():
    global notFoundObjs
    global browserReady
    global sharedArrayBuffer
    browserReady = False
    options = webdriver.ChromeOptions()
    options.add_argument('--headless')
    options.add_argument('--log-level=VERBOSE')
    options.add_argument('--enable-logging')  # Enable logging of console messages
    driver = webdriver.Chrome(options=options)
    driver.get('http://localhost:8080')
    timenow = 0
    while timenow < 5:
        for logtype in driver.log_types:
            logs = driver.get_log(logtype)
            for entry in logs:
                if "SharedArrayBuffer" in entry["message"]:
                    myprint(entry["message"], color="red")
                    sharedArrayBuffer += 1
            captured_messages = driver.execute_script("return consoleLogMessages;")
            for message in captured_messages:
                if "#X: no such object" in message:
                    myprint(message.replace("\n", ""), color="red")
                    notFoundObjs += 1
                driver.execute_script("consoleLogMessages.shift();")
        time.sleep(1)
        timenow += 1

    if notFoundObjs > 0:
        myprint(f"Found {notFoundObjs} errors.", color="red")
        sys.exit(1)
    else:
        myprint("All tests passed.", color="green")
    driver.quit()
    browserReady = True


TestFolder = os.path.dirname(os.path.realpath(__file__))

# Set up Chrome in headless mode
chrome_options = Options()
chrome_options.add_argument('--headless')  # Run in headless mode
chrome_options.add_argument('--enable-logging')  # Enable logging of console messages
driver = webdriver.Chrome(options=chrome_options)



compilationErrors = 0
for root, folders, files in os.walk(TestFolder):
    for folder in folders:
        if folder != ".backup" and folder != "webpatch":
            for root, folders, files in os.walk(os.path.join(TestFolder, folder)):
                for file in files:
                    if file.endswith(".pd") and file == f"{folder}.pd":        
                        testFile = os.path.join(TestFolder, folder, f"{folder}.pd")        
                        fileRoot = os.path.join(TestFolder, folder)
                        os.chdir(fileRoot)
                        thread = threading.Thread(target=compile, args=(testFile,))
                        thread.start()
                        while not compilationReady:
                            time.sleep(1)
                        serverThread = threading.Thread(target=testinBrowser)
                        serverThread.start()
                        time.sleep(1)
                        while not browserReady:
                            time.sleep(1)

                        # kill pd4web
                        os.system("killall pd4web")

                        

if compilationErrors > 0:
    print(f"Found {compilationErrors} errors.")
    sys.exit(1)
else:
    print("All tests passed.")
    sys.exit(0)






