import ctypes


class Error(ctypes.Structure):
    _fields_ = [
        ("code", ctypes.c_char_p),
        ("reason", ctypes.c_char_p),
    ]


class Device(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("timestamp", ctypes.c_uint64),
    ]


class RoomStatus(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("id", ctypes.c_uint64),
        ("num_devices", ctypes.c_uint64),
        ("devices", ctypes.POINTER(Device)),
    ]


class ByteArray(ctypes.Structure):
    _fields_ = [
        ("size", ctypes.c_uint64),
        ("data", ctypes.c_void_p),
    ]


class FileInfo(ctypes.Structure):
    _fields_ = [
        ("filename", ctypes.c_char_p),
        ("data", ByteArray),
    ]


class MultiFile(ctypes.Structure):
    _fields_ = [("num_files", ctypes.c_uint64), ("files", ctypes.POINTER(FileInfo))]


class ContentData(ctypes.Union):
    _fields_ = [
        ("file", ByteArray),
        ("clipboard", ctypes.c_char_p),
        ("multiFile", MultiFile),
    ]


class Content(ctypes.Structure):
    _fields_ = [
        ("device", ctypes.c_char_p),
        ("timestamp", ctypes.c_uint64),
        ("type", ctypes.c_int32),
        ("name", ctypes.c_char_p),
        ("mime", ctypes.c_char_p),
        ("data", ContentData),
    ]
