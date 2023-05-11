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


class ConfigClient:

    rpc: RpcClient

    def __init__(self, rp: RpcClient):
        self.rpc = rp

    @staticmethod
    def relax(key):
        assert isinstance(key, str)
        if len(key) > 0 and key[0] != "/":
            return "/" + key
        else:
            return key

    def config_set(self, key, value):
        return self.rpc.call("config_set", {"key": self.relax(key), "value": value})

    def config_get(self, key):
        return self.rpc.call("config_get", {"key": self.relax(key)})

    def __getattr__(self, key):
        return self.config_get(key)

    def __getitem__(self, item):
        return self.config_get(item)

    def __setitem__(self, key, value):
        pprint(self.config_set(key, value))

    def __repr__(self):
        return pformat(self.config_get(""))


# %%
if __name__ == "__main__":

    # Usage: ipython -i -m remote_tools.client "tcp://192.168.43.217:5555"

    import sys

    if len(sys.argv) > 1:
        address = sys.argv[1]
    else:
        address = "tcp://127.0.0.1:5555"

    r = RpcClient(address)
    c = ConfigClient(r)
