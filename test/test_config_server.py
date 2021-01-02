from pprint import pprint

import yaml
from jsonpointer import resolve_pointer, set_pointer, JsonPointer, JsonPointerException
import msgpack as msp

import asyncio
import zmq
import zmq.asyncio

import sys
sys.modules['cloudpickle'] = None

# url = "ipc://some"
url = "tcp://127.0.0.1:5555"
ctx = zmq.asyncio.Context()


async def recv_and_process():
    sock = ctx.socket(zmq.REP)
    sock.bind(url)
    with open("/home/slovak/awesomeconfig/config/conf_test.yaml") as iff:
        jobj = yaml.safe_load(iff)

    while True:
        msg = await sock.recv_multipart()

        responce = None

        try:
            request = msp.unpackb(msg[0])
            pprint(request)

            if request["type"] == "get":
                responce = resolve_pointer(jobj, request["key"])
            elif request["type"] == "set":
                set_pointer(jobj, request["key"], request["value"])
                responce = True
        except:
            pass

        print(responce)

        await sock.send(msp.packb(responce))

asyncio.run(recv_and_process())

