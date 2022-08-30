#!/usr/bin/env python3
import platform
import unittest
import subprocess as sp
from remote_tools import Client


class TestRemoteConfigInterface(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        if platform.architecture()[0] == "64bit":
            server_exe_file_path = "/home/slovak/remote-config/cmake-build-debug/test/test_config_server"
        else:
            server_exe_file_path = "/home/pi/remote-config/build/test/test_config_server"

        cls.server = sp.Popen([server_exe_file_path])
        cls.c = Client()

    @classmethod
    def tearDownClass(cls):
        cls.server.kill()

    def test_Scalars(self):
        self.assertEqual(self.c.set("ctrl/theta_k", -30), -30)
        self.assertEqual(self.c.set("ctrl/theta_k", -30.0), -30)
        self.assertEqual(self.c.set("ctrl/theta_k", 30.0), 30)
        self.assertEqual(self.c.set("ctrl/theta_k", 30), 30)
        self.assertEqual(self.c.set("ctrl/theta_k", 30.5), 30)

    def test_GetMissuse(self):
        self.assertEqual(self.c.get("fasldjfalseaesf"), {'error': "[json.exception.out_of_range.403] key 'fasldjfalseaesf' not found"})

    def test_SetMissuse(self):
        self.assertEqual(self.c.set("fasldjfalseaesf", 0), {'error': "[json.exception.out_of_range.403] key 'fasldjfalseaesf' not found"})

    def test_VectorFloats(self):
        self.assertEqual(self.c.set("test/matrix/data", [
            1, 1, 2.0,
            1, 1, -2,
            1, 1, -20.0,
        ]), [
            1, 1, 2.0,
            1, 1, -2,
            1, 1, -20.0,
        ])

    def test_VectorInts(self):
        self.assertEqual(self.c.set("test/vector/data/2", 0), 0)
        self.assertEqual(self.c.set("test/vector/data/2", -1), -1)
        self.assertEqual(self.c.set("test/vector/data/2", -1.5), -1)

        self.assertEqual(self.c.set("test/vector/data", [1, 2, 3]), [1, 2, 3])
        self.assertEqual(self.c.set("test/vector/data", [1, 2, -3]), [1, 2, -3])
        self.assertEqual(self.c.set("test/vector/data", [1, 2, 1.5]), [1, 2, 1], "cast float to int")
        self.assertEqual(self.c.set("test/vector/data", [1, 2, -1.5]), [1, 2, -1], "cast float to int")

    def test_VectorIntsMissuse(self):
        self.assertEqual(self.c.set("test/vector/data/2", "str"), {'error': '[json.exception.type_error.302] type must be number, but is string'})
        self.assertEqual(self.c.set("test/vector/data/10", 0), {'error': '[json.exception.out_of_range.401] array index 10 is out of range'})
        self.assertEqual(self.c.set("test/vector/data/0", [0, 0]), {'error': '[json.exception.type_error.302] type must be number, but is null'})
        self.assertEqual(self.c.set("test/vector/data", [0, 0]), {'error': '[json.exception.type_error.302] type must be number, but is null'})


if __name__ == "__main__":
    unittest.main()
