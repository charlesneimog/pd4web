import os
import sys
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import threading 
import multiprocessing
import http.server
import time
import platform
import zipfile


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


def start_server(server):
    server.serve_forever()


def testinBrowser(TestFolder):
    os.chdir(TestFolder)
    myprint(f"Testing {TestFolder} in browser...", color="blue")
    sharedArrayBuffer = 0
    notFoundObjs = 0
    nullErrors = 0
    options = webdriver.ChromeOptions()
    options.add_argument('--headless')
    options.add_argument('--log-level=VERBOSE')
    options.add_argument('--enable-logging')  # Enable logging of console messages
    driver = webdriver.Chrome(options=options)
    server = http.server.HTTPServer(('127.0.0.1', 8080), http.server.SimpleHTTPRequestHandler)
    threading.Thread(target=start_server, args=(server,)).start()
    driver.get('http://127.0.0.1:8080')
    timenow = 0
    while timenow < 10:
        for logtype in driver.log_types:
            logs = driver.get_log(logtype)
            for entry in logs:
                if "SharedArrayBuffer" in entry["message"]:
                    myprint(entry["message"], color="red")
                    sharedArrayBuffer += 1
                elif "Uncaught RuntimeError" in entry["message"]:
                    myprint(entry["message"], color="red")
                    nullErrors += 1
                else:
                    myprint(entry["message"], color="yellow")
            captured_messages = driver.execute_script("return consoleLogMessages;")
            for message in captured_messages:
                if "#X: no such object" in message:
                    myprint(message.replace("\n", ""), color="red")
                    notFoundObjs += 1
                driver.execute_script("consoleLogMessages.shift();")
        time.sleep(1)
        timenow += 1
    if nullErrors > 0:
        myprint(f"Found {notFoundObjs} errors.", color="red")
        driver.quit()
        server.shutdown()
        server.server_close()
        sys.exit(1)
    else:
        myprint("All browser tests passed.", color="green")
    driver.quit()
    server.shutdown()
    server.server_close()
    return nullErrors 

# "compiledWebsite-Win.zip",
if __name__ == '__main__':
    multiprocessing.freeze_support()
    TestFolder = os.path.dirname(os.path.realpath(__file__))
    for root, folders, files in os.walk(TestFolder):
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
                                if platform.system() == "Linux": # just test in browser on Linux
                                   compilationErrors += testinBrowser(fileRoot)                          
        if compilationErrors > 0:
            print(f"Found {compilationErrors} errors.")
            sys.exit(1)
        else:
            print("All tests passed.")
            sys.exit(0)






