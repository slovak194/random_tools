import unittest
import subprocess as sp
from remote_config import ConfigClient

server_exe_file_path = "/home/slovak/remote-config/cmake-build-debug/test/test_server"


class TestMoteusProtocolPython(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.server = sp.Popen([server_exe_file_path])
        cls.c = ConfigClient()

    @classmethod
    def tearDownClass(cls):
        cls.server.kill()

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


if __name__ == '__main__':
    unittest.main()
