import multiprocessing
import os
import pd4web as Pd4Web
import platform


import unittest
import shutil

import shutil
import time


def execute_chrome(path, port):
    from selenium import webdriver
    from selenium.webdriver.chrome.options import Options
    from selenium.webdriver.chrome.service import Service
    from selenium.webdriver.common.by import By
    from flask import Flask, send_from_directory

    app = Flask(__name__, static_url_path="", static_folder=path)
    chrome_options = Options()
    chrome_options.add_argument("--headless=new")
    ChromeDriver = shutil.which("chromedriver")
    if ChromeDriver is None:
        raise Exception("Chromedriver not found")

    # Caminho para o ChromeDriver
    chrome_service = Service(executable_path=ChromeDriver)

    @app.after_request
    def add_coop_coep_headers(response):
        response.headers["Cross-Origin-Opener-Policy"] = "same-origin"
        response.headers["Cross-Origin-Embedder-Policy"] = "require-corp"
        return response

    @app.route("/")
    def index():
        return send_from_directory(path, "index.html")

    @app.route("/testar")
    def testar(localport):
        driver = webdriver.Chrome(service=chrome_service, options=chrome_options)
        driver.get(f"http://localhost:{localport}")
        time.sleep(1)  # Wait

        try:
            element = driver.find_element(By.XPATH, '//*[@id="Pd4WebAudioSwitch"]')

            element.click()
            time.sleep(2)  # run for 5 seconds
            logs = driver.get_log("browser")

            for log_entry in logs:
                message = "".join(log_entry["message"])
                if log_entry["level"] == "SEVERE" and log_entry["source"] == "console-api":
                    raise Exception(message)

        finally:
            driver.quit()

        return "Teste concluído!"

    # Inicie o servidor Flask em um processo separado
    server_process = multiprocessing.Process(target=app.run, kwargs={"debug": False, "use_reloader": False})
    server_process.start()

    try:
        testar(port)
    finally:
        server_process.terminate()
        server_process.join()
        time.sleep(1)  # Ensure the port is released before the next test


def RunTest(patchPath, port):
    string = f"Testing {patchPath}"
    print(f"\n\n\n{'='*len(string)}\n{string}\n{'='*len(string)}")
    this_file = os.path.abspath(__file__)
    pd_file = os.path.join(os.path.dirname(this_file), patchPath)
    Pd4WebInstance = Pd4Web.Pd4Web(Patch=pd_file)
    # Pd4WebInstance.SILENCE = True
    Pd4WebInstance.verbose = True
    Pd4WebInstance.Execute()

    # just for linux,
    if platform.system() == "Linux":
        patchDir = os.path.dirname(pd_file)
        execute_chrome(patchDir, port)
        print("\n\n")


class Pd4WebTest(unittest.TestCase):
    def test_gui(self):
        lib = os.path.join(os.path.dirname(__file__), "Basic/gui")
        all_files = os.listdir(os.path.join(os.path.dirname(__file__), "Basic/gui"))
        pd_files = sorted([obj for obj in all_files if obj.endswith(".pd")])

        for i in range(len(pd_files)):
            file_path = pd_files[i]
            filename = file_path.split("/")[-1]
            patchname = filename.split(".")[0]
            os.makedirs(f"{lib}/gui/", exist_ok=True)
            os.makedirs(f"{lib}/gui/{patchname}", exist_ok=True)
            shutil.copyfile(f"{lib}/{file_path}", f"{lib}/gui/{patchname}/{filename}")
            newpatch = f"{lib}/gui/{patchname}/{filename}"
            try:
                RunTest(newpatch, 5000)
            except:
                ## print in red
                print(f"\033[91mError: {newpatch} -- trying again\033[0m")
                RunTest(newpatch, 5000)
        try:
            shutil.rmtree(f"{lib}/gui")
        except:
            pass


"""

# ╭──────────────────────────────────────╮
# │              Libraries               │
# ╰──────────────────────────────────────╯
class Pd4webElse(unittest.TestCase):
    def test_else(self):
        lib = os.path.join(os.path.dirname(__file__), "Libraries/else")
        all_files = os.listdir(os.path.join(os.path.dirname(__file__), "Libraries/else"))
        pd_files = sorted([obj for obj in all_files if obj.endswith(".pd")])

        for i in range(len(pd_files)):
            file_path = pd_files[i]
            filename = file_path.split("/")[-1]
            patchname = filename.split(".")[0]
            os.makedirs(f"{lib}/else/", exist_ok=True)
            os.makedirs(f"{lib}/else/{patchname}", exist_ok=True)
            shutil.copyfile(f"{lib}/{file_path}", f"{lib}/else/{patchname}/{filename}")
            newpatch = f"{lib}/else/{patchname}/{filename}"
            try:
                RunTest(newpatch, 5000)
            except:
                ## print in red
                print(f"\033[91mError: {newpatch} -- trying again\033[0m")
                RunTest(newpatch, 5000)
        try:
            shutil.rmtree(f"{lib}/else")
        except:
            pass


class Pd4webPmpd(unittest.TestCase):
    def test_pmpd(self):
        lib = os.path.join(os.path.dirname(__file__), "Libraries/pmpd")
        all_files = os.listdir(os.path.join(os.path.dirname(__file__), "Libraries/pmpd"))
        pd_files = sorted([obj for obj in all_files if obj.endswith(".pd")])

        for i in range(len(pd_files)):
            file_path = pd_files[i]
            filename = file_path.split("/")[-1]
            patchname = filename.split(".")[0]
            os.makedirs(f"{lib}/pmpd/", exist_ok=True)
            os.makedirs(f"{lib}/pmpd/{patchname}", exist_ok=True)
            shutil.copyfile(f"{lib}/{file_path}", f"{lib}/pmpd/{patchname}/{filename}")
            newpatch = f"{lib}/pmpd/{patchname}/{filename}"
            try:
                RunTest(newpatch, 5000)
            except:
                print(f"\033[91mError: {newpatch} -- trying again\033[0m")
                RunTest(newpatch, 5000)
                pass

        try:
            shutil.rmtree(f"{lib}/pmpd")
        except:
            pass

"""

if __name__ == "__main__":
    unittest.main(failfast=True)
