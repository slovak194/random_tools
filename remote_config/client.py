from pprint import pprint, pformat
from functools import partial

import zmq
import msgpack as msp


class ConfigClient:
    class __ConfigClient:
        def __init__(self, addr="tcp://127.0.0.1:5555", context=None):
            self.address = addr
            self.context = context or zmq.Context.instance()
            self.socket = self.context.socket(zmq.REQ)
            self.socket.connect(self.address)

        @staticmethod
        def relax(key):
            assert isinstance(key, str)
            if len(key) > 0 and key[0] != "/":
                return "/" + key
            else:
                return key

        def set(self, key, value):
            return self.call("set", {"key": self.relax(key), "value": value})

        def get(self, key):
            return self.call("get", {"key": self.relax(key)})

        def call(self, fun, args=None):

            while self.socket.poll(10, zmq.POLLIN) & zmq.POLLIN:
                _ = self.socket.recv()

            self.socket.send(msp.packb({"fun": fun, "args": args}))
            if self.socket.poll(1000, zmq.POLLIN) == 0:
                return None

            rec = self.socket.recv()
            message = msp.unpackb(rec)
            return message

    instance = None

    def __init__(self, *args, **kwargs):
        if not ConfigClient.instance:
            ConfigClient.instance = ConfigClient.__ConfigClient(*args, *kwargs)

    def __getattr__(self, name):
        try:
            return getattr(self.instance, name)
        except AttributeError as e:
            return partial(self.instance.call, name)

    def __getitem__(self, item):
        return self.instance.get(item)

    def __setitem__(self, key, value):
        pprint(self.instance.set(key, value))

    def __repr__(self):
        return pformat(self.instance.get(""))


# %%
if __name__ == "__main__":

    # Usage: ipython -i -m remote_config.client "tcp://192.168.43.217:5555"

    import sys

    if len(sys.argv) > 1:
        address = sys.argv[1]
    else:
        address = "tcp://127.0.0.1:5555"

    c = ConfigClient(address)
