from pprint import pformat
from collections import UserDict

import zmq
import msgpack as msp


class ConfigClient:
    class __ConfigClient:
        def __init__(self, context=None, url="tcp://127.0.0.1:5555"):
            self.url = url
            self.context = context or zmq.Context.instance()
            self.socket = self.context.socket(zmq.REQ)
            self.socket.connect(self.url)

        @staticmethod
        def relax(key):
            if len(key) > 0 and key[0] != "/":
                return "/" + key
            else:
                return key

        def set(self, key, value):
            key = self.relax(key)
            self.socket.send(msp.packb({"type": "set", "key": key, "value": value}))
            rec = self.socket.recv()
            message = msp.unpackb(rec)
            if message:
                return self.get(key)
            else:
                return None

        def get(self, key):
            key = self.relax(key)
            self.socket.send(msp.packb({"type": "get", "key": key}))
            rec = self.socket.recv()
            message = msp.unpackb(rec)
            return message

    instance = None

    def __init__(self, *args, **kwargs):
        if not ConfigClient.instance:
            ConfigClient.instance = ConfigClient.__ConfigClient(*args, *kwargs)

    def __getattr__(self, name):
        return getattr(self.instance, name)

    def __getitem__(self, item):
        return self.instance.get(item)

    def __setitem__(self, key, value):
        self.instance.set(key, value)

    def __repr__(self):
        return pformat(self.instance.get(""))


class ConfAccessor(UserDict):

    def refresh(self):
        path = getattr(self, 'path', "")
        self.data = ConfigClient().get(path)

    def __getitem__(self, key):
        if not isinstance(key, str):
            key = str(key)

        path = getattr(self, 'path', "")

        if "/" in key and path == "":
            return ConfigClient().get(key)

        else:

            self.data = ConfigClient().get(path + "/" + key)

            new_dict = ConfAccessor()
            new_dict.path = path + "/" + key
            new_dict.data = self.data

            return new_dict

    def __setitem__(self, key, value):
        if not isinstance(key, str):
            key = str(key)

        path = getattr(self, 'path', "")

        if "/" in key and path == "":
            ConfigClient().set(key, value)
        else:
            ConfigClient().set(path + "/" + key, value)

    def __repr__(self):
        self.refresh()
        return pformat(self.data)

    def keys(self):
        self.refresh()
        return self.data.keys()

    @property
    def ptr(self):
        return ConfigClient()

# %%
if __name__ == "__main__":
    c = ConfigClient()
    # c = ConfAccessor()

