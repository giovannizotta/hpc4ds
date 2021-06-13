import glob
import os

if __name__ == '__main__':
    files_dir = 'pct/'
    dynamic_files = glob.glob(os.path.join(files_dir, '*_dynamic'))
    print(len(dynamic_files))
    for f in dynamic_files:
        new_name = f.rstrip('dynamic') + 'static'
        os.rename(f, new_name)

    # dynamic_files = glob.glob(os.path.join(files_dir, '*_dynamic_real'))
    # print(len(dynamic_files))
    # for f in dynamic_files:
    #     new_name = f.rstrip('dynamic_real') + 'dynamic'
    #     os.rename(f, new_name)

    static_files = glob.glob(os.path.join(files_dir, '*'))
    print(len(static_files))
    for f in static_files:
        if not f.endswith('_static') and not f.endswith('_dynamic'):
            new_name = f + '_static'
            os.rename(f, new_name)
