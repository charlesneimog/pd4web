import multiprocessing
import os
import pd4web as Pd4Web
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service
from flask import Flask, send_from_directory, after_this_request
from selenium.webdriver.common.by import By


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

    log = logging.getLogger("werkzeug")
    log.setLevel(logging.ERROR)

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


if __name__ == "__main__":
    multiprocessing.freeze_support()
    test_folder = os.path.dirname(os.path.realpath(__file__))
    chrome_options = Options()
    chrome_options.add_argument("--headless")
    chrome_options.add_argument("--enable-logging")
    this_file = os.path.basename(__file__)
    os.chdir(test_folder)
    for item in os.listdir(test_folder):
        item_path = os.path.join(test_folder, item)
        if os.path.isdir(item_path):
            for file in os.listdir(item_path):
                if file.endswith(".pd"):
                    pd_file = os.path.join(item_path, file)
                    project_folder = os.path.dirname(pd_file)
                    print(f"Testing Project {item} with file {file}...")
                    Pd4WebInstance = Pd4Web.Pd4Web()
                    Pd4WebInstance.Patch = pd_file
                    Pd4WebInstance.Execute()
                    try:
                        execute_chrome(project_folder)
                    except:
                        raise Exception("Test failed")
