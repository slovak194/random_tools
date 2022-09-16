#!/usr/bin/env python3

import os
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
            subprocess.Popen(cmd_sync_all, shell=True).wait()

        time.sleep(1)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Sync src to dst using inotify and rsync')
    parser.add_argument('src', metavar='src', type=str, help='src git repo')
    parser.add_argument('dst', metavar='dst', type=str, help='dst git repo')

    args = parser.parse_args()

    args = vars(args)
    args["src"] = os.path.abspath(args["src"])

    dst = args["dst"]
    src = args["src"]

    local_project_name = src.split("/")[-1]

    remote_user, remote_ip_path = dst.split("@")
    res = remote_ip_path.split(":")

    if len(res) == 2:
        remote_ip, remote_path = res
    elif len(res) == 1:
        remote_ip = res[0]
        remote_path = f"/home/{remote_user}/"

    cmd_sync_all = f"rsync -avz --delete --exclude '.git' --filter=':- {src}/.gitignore' {src} {remote_user}@{remote_ip}:{remote_path}"

    print(cmd_sync_all)
    subprocess.Popen(cmd_sync_all, shell=True).wait()

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
