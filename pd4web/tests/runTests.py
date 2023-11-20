import http.server
import multiprocessing
import os
import platform
import sys
import threading
import time

from selenium import webdriver
from selenium.webdriver.chrome.options import Options


def myprint(str, color=None):
    if color is None:
        print("    " + str)
        return
    if color == "red":
        print("\033[91m" + "    " + str + "\033[0m")
    elif color == "green":
        print("\033[92m" + "    " + str + "\033[0m")
    elif color == "yellow":
        print("\033[93m" + "    " + str + "\033[0m")
    elif color == "blue":
        print("\033[94m" + "    " + str + "\033[0m")
    elif color == "magenta":
        print("\033[95m" + "    " + str + "\033[0m")
    elif color == "cyan":
        print("\033[96m" + "    " + str + "\033[0m")
    else:
        print("    " + str)


def run_pd4web(testFile, return_code):
    return_code.value = os.system(f"pd4web --patch {testFile} --html ../index.html")


def start_server(server):
    server.serve_forever()


def testinBrowser(TestFolder):
    os.chdir(TestFolder)
    sharedArrayBuffer = 0
    notFoundObjs = 0
    options = webdriver.ChromeOptions()
    options.add_argument("--headless")
    options.add_argument("--log-level=VERBOSE")
    options.add_argument("--enable-logging")  # Enable logging of console messages
    driver = webdriver.Chrome(options=options)
    server = http.server.HTTPServer(
        ("127.0.0.1", 8080), http.server.SimpleHTTPRequestHandler
    )
    threading.Thread(target=start_server, args=(server,)).start()
    driver.get("http://127.0.0.1:8080")
    timenow = 0
    while timenow < 5:
        for logtype in driver.log_types:
            logs = driver.get_log(logtype)
            for entry in logs:
                if "SharedArrayBuffer" in entry["message"]:
                    myprint(entry["message"], color="red")
                    sharedArrayBuffer += 1
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
    if notFoundObjs > 0:
        myprint(f"Found {notFoundObjs} errors.", color="red")
        sys.exit(1)
    else:
        myprint("All browser tests passed.", color="green")
    driver.quit()
    server.shutdown()
    server.server_close()
    return 0


if __name__ == "__main__":
    multiprocessing.freeze_support()
    TestFolder = os.path.dirname(os.path.realpath(__file__))
    chrome_options = Options()
    chrome_options.add_argument("--headless")  # Run in headless mode
    chrome_options.add_argument(
        "--enable-logging"
    )  # Enable logging of console messages
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
                            return_code = multiprocessing.Value("i", 0)
                            pd4web_process = multiprocessing.Process(
                                target=run_pd4web, args=(testFile, return_code)
                            )
                            pd4web_process.start()
                            pd4web_process.join()
                            # check if there is return_value in return_code
                            exit_code = return_code.value
                            if exit_code != 0:
                                myprint(f"Found {exit_code} errors.", color="red")
                                compilationErrors += 1
                            if (
                                platform.system() == "Linux"
                            ):  # just test in browser on Linux
                                testinBrowser(fileRoot)

    if compilationErrors > 0:
        print(f"Found {compilationErrors} errors.")
        sys.exit(1)
    else:
        print("All tests passed.")
        sys.exit(0)
