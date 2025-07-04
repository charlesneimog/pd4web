# test_pd4web_compile_selenium.py

import os
import sys
import time
from pathlib import Path
import pytest
import pd4web
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
import shutil
import multiprocessing
from flask import Flask, send_from_directory


@pytest.fixture(scope="module")
def pd4web_home():
    if sys.platform.startswith("darwin") or sys.platform.startswith("linux"):
        home = os.getenv("HOME")
        if not home:
            pytest.fail("HOME environment variable not set")
        return Path(home) / ".local" / "share" / "pd4web"
    else:
        appdata = os.getenv("APPDATA")
        if not appdata:
            pytest.fail("APPDATA environment variable not set")
        return Path(appdata) / "pd4web"


@pytest.fixture(scope="module")
def compiler(pd4web_home):
    pd4web_home.mkdir(parents=True, exist_ok=True)
    script_path = Path(__file__).parent
    output_folder = script_path / "tmp"
    if output_folder.exists():
        shutil.rmtree(output_folder)
    output_folder.mkdir(parents=True, exist_ok=True)

    comp = pd4web.pypd4web.Pd4Web(str(pd4web_home))
    comp.setOutputFolder(str(output_folder))
    comp.setDebugMode(True)
    comp.setFailFast(True)
    # Ajuste para seu caminho local Pd4Web sources, modifique se necessário
    comp.setPd4WebFilesFolder("/home/neimog/Documents/Git/pd4web/Sources/Pd4Web")
    comp.init()

    yield comp

    # Cleanup
    if output_folder.exists():
        shutil.rmtree(output_folder)


def run_flask_server(directory, port):
    app = Flask(__name__, static_folder=directory, static_url_path="")

    @app.after_request
    def add_headers(response):
        response.headers["Cross-Origin-Opener-Policy"] = "same-origin"
        response.headers["Cross-Origin-Embedder-Policy"] = "require-corp"
        return response

    @app.route("/")
    def index():
        return send_from_directory(directory, "index.html")

    app.run(port=port, debug=False, use_reloader=False)


@pytest.mark.parametrize(
    "patch",
    [
        Path("Basic/abs/main.pd"),
        Path("Basic/audio/main.pd"),
        Path("Basic/declare/main.pd"),
        Path("Basic/declare/main.pd"),
        Path("Basic/else-vs-cyclone/main.pd"),
        Path("Basic/gui/main.pd"),
        Path("Basic/gui2/main.pd"),
        Path("Basic/messages/main.pd"),
        Path("Basic/vsl/main.pd"),
        Path("Basic/vu/main.pd"),
        # Adicione mais patches aqui
    ],
)
def test_compile_and_open_patch(compiler, patch):
    full_path = Path(__file__).parent / patch
    assert full_path.exists(), f"Patch not found: {full_path}"
    compiler.setPatchFile(str(full_path))
    compiler.processPatch()

    script_path = Path(__file__).parent
    output_dir = script_path / "tmp"
    port = 5000

    return

    # Start Flask server in another process
    server_process = multiprocessing.Process(target=run_flask_server, args=(output_dir, port))
    server_process.start()
    time.sleep(2)  # Wait for server to start

    try:
        chrome_driver_path = shutil.which("chromedriver")
        assert chrome_driver_path, "chromedriver not found in PATH"

        options = Options()
        options.add_argument("--headless=new")
        options.add_argument("--disable-gpu")
        options.add_argument("--no-sandbox")
        options.add_argument("--disable-dev-shm-usage")

        service = Service(executable_path=chrome_driver_path)
        driver = webdriver.Chrome(service=service, options=options)
        driver.get(f"http://localhost:{port}")

        # Basic validation: título da página ou algum elemento do Pd4Web
        assert "Pd4Web" in driver.title or "index" in driver.page_source.lower()

        # Se existir um botão ou elemento esperado, você pode verificar
        # Exemplo, elemento de áudio
        try:
            audio_switch = driver.find_element(By.ID, "Pd4WebAudioSwitch")
            audio_switch.click()
        except Exception:
            pass

        time.sleep(2)

        # Checar logs do navegador pode ser implementado se quiser

        driver.quit()
    finally:
        server_process.terminate()
        server_process.join()
        time.sleep(1)
