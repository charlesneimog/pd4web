import os
import shutil

def earplug_extra(librarySelf):
    ''' 
    This function copy some things that I already need to compile some externals in cyclone
    '''
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.ROOT, "src", "externals")):
        os.makedirs(os.path.join(librarySelf.ROOT, "src", "externals"))

    if not os.path.exists(os.path.join(librarySelf.ROOT, "src", "extra")):
        os.makedirs(os.path.join(librarySelf.ROOT, "src", "extra"))

    if not os.path.exists(os.path.join(librarySelf.ROOT, "src", "data")):
        os.makedirs(os.path.join(librarySelf.ROOT, "src", "data"))

    folder = librarySelf.folder
    shutil.copy(os.path.join(folder, "earplug_data.txt"), os.path.join(librarySelf.ROOT, "src", "data/earplug_data.txt"))
    
            
    with open(os.path.join(folder, "earplug~.c"), "r") as f:
        lines = f.readlines()
        for i in range(len(lines)):
            if 'filedesc = open_via_path(canvas_getdir(canvas_getcurrent())->s_name, "earplug_data.txt", "", buff, &bufptr, 1024, 0 );' in lines[i]:
                lines[i] = 'filedesc = open_via_path(canvas_getdir(canvas_getcurrent())->s_name, "data/earplug_data.txt", "", buff, &bufptr, 1024, 0 );\n'
                break
    with open(os.path.join(librarySelf.ROOT, "src", "externals", "earplug~.c"), "w") as f:
        f.writelines(lines)


    

 
    




