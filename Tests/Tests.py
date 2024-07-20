import multiprocessing
import os
import pd4web as Pd4Web
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service
from flask import Flask, send_from_directory, after_this_request
from selenium.webdriver.common.by import By

import unittest
import re

import shutil
from pprint import pprint
import logging


def execute_chrome(path):
    app = Flask(__name__, static_url_path="", static_folder=path)
    chrome_options = Options()
    chrome_options.add_argument("--headless=new")
    ChromeDriver = shutil.which("chromedriver")
    if ChromeDriver is None:
        raise Exception("Chromedriver not found")

    # Caminho para o ChromeDriver
    chrome_service = Service(executable_path=ChromeDriver)

    # Suppress Flask's default logging
    # log = logging.getLogger("werkzeug")
    # log.setLevel(logging.ERROR)
    # app.logger.setLevel(logging.ERROR)

    @app.after_request
    def add_coop_coep_headers(response):
        response.headers["Cross-Origin-Opener-Policy"] = "same-origin"
        response.headers["Cross-Origin-Embedder-Policy"] = "require-corp"
        return response

    @app.route("/")
    def index():
        return send_from_directory(path, "index.html")

    @app.route("/testar")
    def testar():
        driver = webdriver.Chrome(
            service=chrome_service, options=chrome_options)
        driver.get("http://localhost:5000")

        element = driver.find_element(By.XPATH, '//*[@id="turnAudioOn"]')
        element.click()
        logs = driver.get_log("browser")

        for log_entry in logs:
            message = "".join(log_entry["message"])
            if log_entry["level"] == "SEVERE" and log_entry["source"] == "console-api":
                driver.quit()
                server_process.terminate()
                server_process.join()
                raise Exception(message)

        driver.quit()
        return "Teste concluído!"

    # Inicie o servidor Flask em um processo separado
    server_process = multiprocessing.Process(
        target=app.run, kwargs={"debug": False, "use_reloader": False})
    server_process.start()

    # Aguarde um pouco para garantir que o servidor Flask está rodando
    import time

    time.sleep(1)
    testar()

    # Finalize o servidor Flask
    server_process.terminate()
    server_process.join()


class TestMyModule(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        # search for .git folders and remove them
        for root, dirs, files in os.walk(os.path.dirname(os.path.abspath(__file__))):
            for dir in dirs:
                if dir == ".git":
                    shutil.rmtree(os.path.join(root, dir))

    def RunTest(self, number):
        print()
        print("# ╭──────────────────────────────────────╮")
        print(f"# │               Test {number:03}               │")
        print("# ╰──────────────────────────────────────╯")
        print("")

        root = os.path.dirname(os.path.abspath(__file__))
        pd_file = os.path.join(root, f"Test{number}", f"Test{number}.pd")
        Pd4WebInstance = Pd4Web.Pd4Web()
        Pd4WebInstance.Patch = pd_file
        # Pd4WebInstance.Silence()
        Pd4WebInstance.Execute()
        execute_chrome(f"Test{number}")
        print("\n\n")

    def test_1(self):
        self.RunTest(1)

    def test_2(self):
        self.RunTest(2)

    def test_3(self):
        self.RunTest(3)

    def test_4(self):
        with self.assertRaises(ValueError) as context:
            self.RunTest(4)  # Use a number that you know doesn't exist
        self.assertEqual(str(context.exception), "Patch not found")

    def test_5(self):
        with self.assertRaises(ValueError) as context:
            self.RunTest(5)
        self.assertEqual(str(context.exception),
                         "Library not supported: notfound")


if __name__ == "__main__":
    unittest.main()
