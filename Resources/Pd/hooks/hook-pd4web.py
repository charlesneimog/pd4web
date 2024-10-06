from PyInstaller.utils.hooks import collect_all

# Collect all files related to the cmake module
datas, binaries, hiddenimports = collect_all("pd4web")
