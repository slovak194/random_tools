from pprint import pprint, pformat

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
            assert isinstance(key, str)
            if len(key) > 0 and key[0] != "/":
                return "/" + key
            else:
                return key

        def set(self, key, value):
            key = self.relax(key)

            while self.socket.poll(10, zmq.POLLIN) & zmq.POLLIN:
                _ = self.socket.recv()

            self.socket.send(msp.packb({"cmd": "set", "key": key, "value": value}))

            if self.socket.poll(1000, zmq.POLLIN) == 0:
                return None

            rec = self.socket.recv()
            message = msp.unpackb(rec)
            return message

        def get(self, key):
            key = self.relax(key)

            while self.socket.poll(10, zmq.POLLIN) & zmq.POLLIN:
                _ = self.socket.recv()

            self.socket.send(msp.packb({"cmd": "get", "key": key}))

            if self.socket.poll(1000, zmq.POLLIN) == 0:
                return None

            rec = self.socket.recv()

            message = msp.unpackb(rec)
            return message

        def load(self, value=""):
            self.socket.send(msp.packb({"cmd": "load", "value": value}))
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
        pprint(self.instance.set(key, value))

    def __repr__(self):
        return pformat(self.instance.get(""))


# %%
if __name__ == "__main__":

    # Usage: ipython -m remote_config.client -i

    c = ConfigClient()

