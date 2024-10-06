from PyInstaller.utils.hooks import collect_data_files, collect_all

datas, binaries, hiddenimports = collect_all('cmake')

# Additionally ensure data files (if any are missed)
datas += collect_data_files('cmake')