import multiprocessing
import os
import pd4web as Pd4Web
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service
from flask import Flask, send_from_directory
from selenium.webdriver.common.by import By

import unittest
import threading
import shutil

import shutil
import time


def execute_chrome(path, port):
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
        driver = webdriver.Chrome(
            service=chrome_service, options=chrome_options)
        driver.get(f"http://localhost:{localport}")
        time.sleep(1)  # Wait

        try:
            element = driver.find_element(
                By.XPATH, '//*[@id="Pd4WebAudioSwitch"]')

            element.click()
            time.sleep(5)  # run for 5 seconds
            logs = driver.get_log("browser")

            for log_entry in logs:
                message = "".join(log_entry["message"])
                if log_entry["level"] == "SEVERE" and log_entry["source"] == "console-api":
                    raise Exception(message)

        finally:
            driver.quit()

        return "Teste concluído!"

    # Inicie o servidor Flask em um processo separado
    server_process = multiprocessing.Process(
        target=app.run, kwargs={"debug": False, "use_reloader": False})
    server_process.start()

    try:
        testar(port)
    finally:
        server_process.terminate()
        server_process.join()
        time.sleep(1)  # Ensure the port is released before the next test


class TestMyModule(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        for root, dirs, files in os.walk(os.path.dirname(os.path.abspath(__file__))):
            for file in files:
                if file == "CMakeLists.txt":
                    os.remove(os.path.join(root, file))
                if file == "favicon.ico":
                    os.remove(os.path.join(root, file))
                if file == "index.html":
                    os.remove(os.path.join(root, file))
            for dir in dirs:
                if dir == ".git":
                    shutil.rmtree(os.path.join(root, dir))
                if dir == "WebPatch":
                    shutil.rmtree(os.path.join(root, dir))
                if dir == "Pd4Web":
                    shutil.rmtree(os.path.join(root, dir))
                if dir == "build":
                    shutil.rmtree(os.path.join(root, dir))

    def RunTest(self, patchPath, port):
        string = f"Testing {patchPath}"
        print(f"\n\n\n{'='*len(string)}\n{string}\n{'='*len(string)}")

        this_file = os.path.abspath(__file__)
        pd_file = os.path.join(os.path.dirname(this_file), patchPath)
        Pd4WebInstance = Pd4Web.Pd4Web(Patch=pd_file)
        # Pd4WebInstance.SILENCE = True
        Pd4WebInstance.verbose = True
        Pd4WebInstance.Execute()

        patchDir = os.path.dirname(pd_file)
        execute_chrome(patchDir, port)
        print("\n\n")

    def test_ObjectNotFound(self):
        with self.assertRaises(ValueError) as context:
            self.RunTest("Basic/objnotfound.pd", 5000)
        if "Object not found:" not in str(context.exception):
            raise Exception("Object not found not raised")

    def test_pmpd(self):
        self.RunTest("Libraries/pmpd-noaudio.pd", 5000)

        with self.assertRaises(ValueError) as context:
            self.RunTest("Libraries/pmpd-audio.pd", 5000)
        if "not supported by Pd4Web yet." not in str(context.exception):
            raise Exception("pmpd not raised")

    def test_else(self):
        lib = os.path.join(os.path.dirname(__file__), "Libraries/else")
        all_files = os.listdir(os.path.join(
            os.path.dirname(__file__), "Libraries/else"))
        pd_files = sorted([obj for obj in all_files if obj.endswith(".pd")])

        for i in range(len(pd_files)):
            file_path = pd_files[i]
            filename = file_path.split("/")[-1]
            patchname = filename.split(".")[0]
            os.makedirs(f"{lib}/else/", exist_ok=True)
            os.makedirs(f"{lib}/else/{patchname}", exist_ok=True)
            shutil.copyfile(f"{lib}/{file_path}",
                            f"{lib}/else/{patchname}/{filename}")
            newpatch = f"{lib}/else/{patchname}/{filename}"
            try:
                self.RunTest(newpatch, 5000)
            except:
                pass

            self.RunTest(newpatch, 5000)

        shutil.rmtree(f"{lib}/else")


if __name__ == "__main__":
    unittest.main()
