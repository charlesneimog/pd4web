import multiprocessing
import os
import pd4web as Pd4Web
import platform

import unittest
import shutil

import shutil
import time


class Pd4WebTest(unittest.TestCase):
    def execute_server(self, patchPath, port):
        string = f"Testing {patchPath}"
        print(f"\n\n\n{'='*len(string)}\n{string}\n{'='*len(string)}")
        this_file = os.path.abspath(__file__)
        pd_file = os.path.join(os.path.dirname(this_file), patchPath)
        Pd4WebInstance = Pd4Web.Pd4Web(Patch=pd_file)
        Pd4WebInstance.verbose = True
        Pd4WebInstance.Execute()
        if platform.system() == "Linux":
            patchDir = os.path.dirname(pd_file)
            self.execute_chrome(patchDir, port)
            print("\n\n")

    def execute_chrome(self, path, port):
        from selenium import webdriver
        from selenium.webdriver.chrome.options import Options
        from selenium.webdriver.chrome.service import Service
        from selenium.webdriver.common.by import By
        from flask import Flask, send_from_directory

        app = Flask(__name__, static_url_path="", static_folder=path)
        chrome_options = Options()
        chrome_options.add_argument("--headless=new")
        chrome_options.add_argument("--disable-gpu")
        chrome_options.add_argument("--no-sandbox")
        chrome_options.add_argument("--disable-dev-shm-usage")
        ChromeDriver = shutil.which("chromedriver")
        chrome_options.set_capability("goog:loggingPrefs", {"browser": "ALL", "performance": "ALL"})
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
            element = driver.find_element(By.XPATH, '//*[@id="Pd4WebAudioSwitch"]')
            element.click()
            time.sleep(2)  # run for 5 seconds
            logs = driver.get_log("browser")
            prev_msg = ""
            print("")
            for log_entry in logs:
                message = "".join(log_entry["message"])
                print(message)
                if log_entry["level"] == "SEVERE" and log_entry["source"] == "console-api":
                    raise Exception(message)
                if "error: ... couldn't create" in message:
                    raise Exception(message + " " + prev_msg)
                prev_msg = message

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
            time.sleep(3)  # Ensure the port is released before the next test

    def errors(self, directory):
        temp_file = directory.split("/")[-1]
        lib = os.path.join(os.path.dirname(__file__), directory)
        all_files = os.listdir(os.path.join(os.path.dirname(__file__), directory))
        pd_files = sorted([obj for obj in all_files if obj.endswith(".pd")])
        error_dict = {
            "objnotfound": "Library or Object can't be processed, please report",
        }

        for i in range(len(pd_files)):
            file_path = pd_files[i]
            filename = file_path.split("/")[-1]
            patchname = filename.split(".")[0]
            os.makedirs(f"{lib}/{temp_file}", exist_ok=True)
            os.makedirs(f"{lib}/{temp_file}/{patchname}", exist_ok=True)
            shutil.copyfile(f"{lib}/{file_path}", f"{lib}/{temp_file}/{patchname}/{filename}")
            newpatch = f"{lib}/{temp_file}/{patchname}/{filename}"
            try:
                self.execute_server(newpatch, 5000)
                raise Exception("Error not found")
            except Exception as e:
                for key, value in error_dict.items():
                    if value in str(e):
                        print(f"Passed: {newpatch}")
                        break
                else:
                    shutil.rmtree(f"{lib}/{temp_file}")
                    raise Exception(f"Please check {newpatch} for errors")
        try:
            shutil.rmtree(f"{lib}/{temp_file}")
        except:
            pass

    def libraries(self, directory):
        temp_file = directory.split("/")[-1]
        lib = os.path.join(os.path.dirname(__file__), directory)
        all_files = os.listdir(os.path.join(os.path.dirname(__file__), directory))
        pd_files = sorted([obj for obj in all_files if obj.endswith(".pd")])
        for file_path in pd_files:
            filename = file_path.split("/")[-1]
            os.makedirs(f"{lib}/{temp_file}", exist_ok=True)

            src_dir = lib
            dst_dir = os.path.join(lib, temp_file)
            if os.path.exists(dst_dir):
                shutil.rmtree(dst_dir)

            shutil.copytree(src_dir, dst_dir, ignore=shutil.ignore_patterns("*.git", ".git"))
            newpatch = f"{lib}/{temp_file}/{filename}"
            try:
                self.execute_server(newpatch, 5000)
                shutil.rmtree(f"{lib}/{temp_file}")
            except Exception as e:
                shutil.rmtree(f"{lib}/{temp_file}")
                raise Exception(f"Please check {newpatch} for errors")
        try:
            shutil.rmtree(f"{lib}/{temp_file}")
        except:
            pass

    def test_libraries(self):
        # Errors
        self.errors("Basic/errors")

        # Basic
        self.libraries("Basic/abs")
        self.libraries("Basic/audio")
        self.libraries("Basic/declare")
        self.libraries("Basic/file")
        self.libraries("Basic/gui")

        # Libraries
        self.libraries("Libraries/cyclone")
        self.libraries("Libraries/else")
        self.libraries("Libraries/pmpd")
        self.libraries("Libraries/o.scofo~")
        # self.libraries("Libraries/ambi~")
        
        # Issues
        self.libraries("Issues/#22-1")
        self.libraries("Issues/#22-2")

        


if __name__ == "__main__":
    unittest.main(failfast=True)
