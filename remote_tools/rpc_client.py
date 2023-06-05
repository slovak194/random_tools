from pprint import pprint, pformat
from functools import partial

import zmq
import msgpack as msp


class RpcClient:
    def __init__(self, addr="tcp://127.0.0.1:5555", context=None):
        self.address = addr
        self.context = context or zmq.Context.instance()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect(self.address)

    def call(self, fun, args=None):
        while self.socket.poll(10, zmq.POLLIN) & zmq.POLLIN:
            _ = self.socket.recv()

        self.socket.send(msp.packb({"fun": fun, "args": args}))
        if self.socket.poll(1000, zmq.POLLIN) == 0:
            return None

        rec = self.socket.recv()
        message = msp.unpackb(rec)
        return message

    def __repr__(self):
        return pformat(self.call("list"))

    def __call__(self, *args, **kwargs):
        return self.call(*args)

    def __getattr__(self, method):
        return lambda **kwargs: self.call(method, kwargs)


# %%
if __name__ == "__main__":

    # Usage: ipython -i -m remote_tools.client "tcp://192.168.43.217:5555"

    import sys

    if len(sys.argv) > 1:
        address = sys.argv[1]
    else:
        address = "tcp://127.0.0.1:5555"

    r = RpcClient(address)
