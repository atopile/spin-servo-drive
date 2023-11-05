from .constants import CAN_EFF_FLAG, CAN_RTR_FLAG, CAN_ERR_FLAG, CAN_EFF_MASK
from struct import *

# gs_usb general
GS_USB_ECHO_ID = 0
GS_USB_NONE_ECHO_ID = 0xFFFFFFFF

GS_USB_FRAME_SIZE = 20
GS_USB_FRAME_SIZE_HW_TIMESTAMP = 24

class GsUsbFrame:
    def __init__(self, can_id=0, data=[]):
        self.echo_id = GS_USB_ECHO_ID
        self.can_id = can_id
        self.channel = 0
        self.flags = 0
        self.reserved = 0
        self.timestamp_us = 0

        if type(data) == bytes:
            data = list(data)

        self.data = data + [0] * (8 - len(data))
        self.can_dlc = len(data)

    @property
    def arbitration_id(self) -> int:
        return self.can_id & CAN_EFF_MASK

    @property
    def is_extended_id(self) -> bool:
        return bool(self.can_id & CAN_EFF_FLAG)

    @property
    def is_remote_frame(self) -> bool:
        return bool(self.can_id & CAN_RTR_FLAG)

    @property
    def is_error_frame(self) -> bool:
        return bool(self.can_id & CAN_ERR_FLAG)

    @property
    def timestamp(self):
        return self.timestamp_us / 1000000.0

    def __sizeof__(self, hw_timestamp):
        if (hw_timestamp == True):
            return GS_USB_FRAME_SIZE_HW_TIMESTAMP
        else:
            return GS_USB_FRAME_SIZE

    def __str__(self) -> str:
        data = (
            "remote request"
            if self.is_remote_frame
            else " ".join("{:02X}".format(b) for b in self.data[:self.can_dlc])
        )
        return "{: >8X}   [{}]  {}".format(self.arbitration_id, self.can_dlc, data)

    def pack(self, hw_timestamp):
        if (hw_timestamp == True):
            return pack("<2I12BI",
                self.echo_id, self.can_id, self.can_dlc, self.channel,
                self.flags, self.reserved, *self.data, self.timestamp_us
            )
        else:
            return pack("<2I12B",
                self.echo_id, self.can_id, self.can_dlc, self.channel,
                self.flags, self.reserved, *self.data,
            )

    @staticmethod
    def unpack_into(frame, data: bytes, hw_timestamp):
        if (hw_timestamp == True):
            (
                frame.echo_id, frame.can_id, frame.can_dlc, frame.channel,
                frame.flags, frame.reserved, *frame.data, frame.timestamp_us,
            ) = unpack("<2I12BI", data)
        else:
            (
                frame.echo_id, frame.can_id, frame.can_dlc, frame.channel,
                frame.flags, frame.reserved, *frame.data,
            ) = unpack("<2I12B", data)
