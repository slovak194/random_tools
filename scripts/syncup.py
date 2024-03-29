#!/usr/bin/env python3

# TODO, deal with non standard ports. Maybe just pass cmd to be run on trigger from inotify.

import os
import sys
import threading
import argparse
import subprocess
import inotify.adapters
import inotify.constants as ic
import time

dirty = False
cmd_sync_all = ""


def worker():
    global dirty, cmd_sync_all
    while True:
        if dirty:
            dirty = False
            print(cmd_sync_all)

            process = subprocess.Popen(cmd_sync_all, shell=True)
            process.communicate()
            if process.returncode != 0:
                print("Exiting ...")
                sys.exit(1)

        time.sleep(0.5)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Sync src to dst using inotify and rsync')
    parser.add_argument('src', metavar='src', type=str, help='src git repo')
    parser.add_argument('dst', metavar='dst', type=str, help='dst git repo')

    args = parser.parse_args()

    args = vars(args)
    args["src"] = os.path.abspath(args["src"])

    dst = args["dst"]
    src = args["src"]

    # rsync -avz --delete --exclude='.git' --filter=':- /home/slovak/kernel-xavier/.gitignore' /home/slovak/kernel-xavier nvidia@nx-devkit-slovak.local:/home/nvidia/
    cmd_sync_all = f"rsync -avz --delete --exclude='.git' --filter=':- {src}/.gitignore' {src} {dst}"

    print(cmd_sync_all)

    process = subprocess.Popen(cmd_sync_all, shell=True)
    process.communicate()
    if process.returncode != 0:
        print("Exiting ...")
        sys.exit(1)

    th = threading.Thread(target=worker).start()

    i = inotify.adapters.InotifyTree(src, mask=(ic.IN_CREATE | ic.IN_MODIFY | ic.IN_DELETE))

    for event in i.event_gen(yield_nones=False):
        (_, type_names, path, filename) = event
        if "~" in filename:
            continue
        if "/." in path:
            continue
        if "build" in path:
            continue

        dirty = True
