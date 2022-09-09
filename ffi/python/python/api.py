import os
import socket
import time
import ctypes
from api_structures import *


class ApiError(Exception):
    def __init__(self, code, reason):
        super().__init__(
            f"An error occurred in the API. code = {code}; reason = {reason}"
        )


class Api:
    def __init__(self, token: str, origin: str, baseUrl: str):
        self.token = token
        self.origin = origin
        self.baseUrl = baseUrl

        dir = os.path.dirname(os.path.abspath(__file__))
        lib = dir + "/libdynoctane.dylib"
        if os.name == "nt":
            self.api = ctypes.windll.LoadLibrary(lib)
        else:
            self.api = ctypes.cdll.LoadLibrary(lib)

        self.__register_function_info()

    def __enter__(self):
        self.key = self.api.octane_api_client__init(
            ctypes.c_char_p(self.token.encode("utf-8")),
            ctypes.c_char_p(self.origin.encode("utf-8")),
            ctypes.c_char_p(self.baseUrl.encode("utf-8")),
        )
        if self.key == None:
            self.__raise_api_error()
        return self

    def __exit__(self, ex_type, ex_value, trace) -> None:
        self.api.octane_api_client__destroy(self.key)
        print("exit: ", ex_type, ex_value, trace)

    def __register_function_info(self) -> None:
        self.api.octane_api_client__get_last_error.argtypes = []
        self.api.octane_api_client__get_last_error.restype = Error

        self.api.octane_api_client__init.argtypes = [
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
        ]
        self.api.octane_api_client__init.restype = ctypes.c_void_p

        self.api.octane_api_client__destroy.argtypes = [ctypes.c_void_p]
        self.api.octane_api_client__destroy.restype = None

        self.api.octane_api_client__create_room.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
        ]
        self.api.octane_api_client__create_room.restype = ctypes.c_uint64

        self.api.octane_api_client__connect_room.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint64,
            ctypes.c_char_p,
        ]
        self.api.octane_api_client__connect_room.restype = ctypes.c_bool

        self.api.octane_api_client__disconnect_room.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint64,
            ctypes.c_char_p,
        ]
        self.api.octane_api_client__disconnect_room.restype = ctypes.c_bool

        self.api.octane_api_client__get_room_status.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint64,
        ]
        self.api.octane_api_client__get_room_status.restype = ctypes.POINTER(RoomStatus)

        self.api.octane_api_client__free_room_status_structure.argtypes = [
            ctypes.POINTER(RoomStatus)
        ]
        self.api.octane_api_client__free_room_status_structure.restype = None

        self.api.octane_api_client__delete_room.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint64,
        ]
        self.api.octane_api_client__delete_room.restype = ctypes.c_bool

        self.api.octane_api_client__get_content.argtypes = [ctypes.c_void_p]
        self.api.octane_api_client__get_content.restype = ctypes.POINTER(Content)

        self.api.octane_api_client__free_content_structure.argtypes = [
            ctypes.POINTER(Content)
        ]
        self.api.octane_api_client__free_content_structure.restype = None

        self.api.octane_api_client__delete_content.argtypes = [ctypes.c_void_p]
        self.api.octane_api_client__delete_content.restype = ctypes.c_bool

        self.api.octane_api_client__upload_content.argtypes = [
            ctypes.c_void_p,
            Content,
        ]
        self.api.octane_api_client__upload_content.restype = ctypes.c_bool

    def __raise_api_error(self) -> None:
        error = self.api.octane_api_client__get_last_error()
        raise ApiError(error.code, error.reason)

    def create_room(self, roomName: str) -> int:
        id = self.api.octane_api_client__create_room(
            self.key,
            ctypes.c_char_p(roomName.encode("utf-8")),
        )
        if id == 0:
            self.__raise_api_error()
        return id

    def connect_room(self, id: int) -> None:
        res = self.api.octane_api_client__connect_room(
            self.key,
            ctypes.c_uint64(id),
            ctypes.c_char_p(socket.gethostname().encode("utf-8")),
        )
        if res == False:
            self.__raise_api_error()

    def disconnect_room(self, id: int, name: str) -> None:
        res = self.api.octane_api_client__disconnect_room(
            self.key,
            ctypes.c_uint64(id),
            ctypes.c_char_p(name.encode("utf-8")),
        )
        if res == False:
            self.__raise_api_error()

    def get_room_status(self, id: int) -> RoomStatus:
        res = self.api.octane_api_client__get_room_status(
            self.key,
            ctypes.c_uint64(id),
        )
        if res == False:
            self.__raise_api_error()
        return res

    def delete_room(self, id: int) -> None:
        res = self.api.octane_api_client__delete_room(
            self.key,
            ctypes.c_uint64(id),
        )
        if res == False:
            self.__raise_api_error()

    def get_content(self) -> Content:
        res = self.api.octane_api_client__get_content(self.key)

        if not res:
            self.__raise_api_error()
        return res.contents

    def delete_content(self) -> None:
        res = self.api.octane_api_client__delete_content(self.key)
        if res == False:
            self.__raise_api_error()

    def upload_content(self, content: Content) -> None:
        res = self.api.octane_api_client__upload_content(self.key, content)
        if res == False:
            self.__raise_api_error()

    # utility methods for uploading text content.
    def upload_text_content(self, text: str) -> None:
        content = Content()
        content.device = ctypes.c_char_p(socket.gethostname().encode("utf-8"))
        content.timestamp = ctypes.c_uint64(int(time.time()))
        # 0 = File, 1 = Clipboard(Text), 2 = Multi file
        content.type = ctypes.c_int32(1)
        content.name = ctypes.c_char_p("".encode("utf-8"))
        content.mime = ctypes.c_char_p("text/plain".encode())
        content.data = ContentData()
        content.data.clipboard = ctypes.c_char_p(text.encode("utf-8"))
        self.upload_content(content)
