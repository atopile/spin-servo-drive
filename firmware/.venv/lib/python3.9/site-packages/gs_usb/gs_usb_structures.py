from struct import *

class DeviceMode:
    def __init__(self, mode, flags):
        self.mode = mode
        self.flags = flags

    def __str__(self):
        return "Mode: %u\r\n"\
            "Flags: 0x%08x\r\n"\
            %(self.mode, self.flags)

    def pack(self):
        return pack("<II", self.mode, self.flags)


class DeviceBitTiming:
    def __init__(self, prop_seg, phase_seg1, phase_seg2, sjw, brp):
        self.prop_seg = prop_seg
        self.phase_seg1 = phase_seg1
        self.phase_seg2 = phase_seg2
        self.sjw = sjw
        self.brp = brp

    def __str__(self):
        return "Prop Seg: %u\r\n"\
            "Phase Seg 1: %u\r\n"\
            "Phase Seg 2: %u\r\n"\
            "SJW: %u\r\n"\
            "BRP: %u\r\n"\
            %(self.prop_seg, self.phase_seg1, self.phase_seg2, self.sjw, self.brp)

    def pack(self):
        return pack("<5I", self.prop_seg, self.phase_seg1, self.phase_seg2, self.sjw, self.brp)


class DeviceInfo:
    def __init__(self, reserved1, reserved2, reserved3, icount, fw_version, hw_version):
        self.reserved1 = reserved1
        self.reserved2 = reserved2
        self.reserved3 = reserved3
        self.icount = icount
        self.fw_version = fw_version
        self.hw_version = hw_version

    def __str__(self):
        return "iCount: %u\r\n"\
            "FW Version: %u\r\n"\
            "HW Version: %u\r\n"\
            %(self.icount, str(self.fw_version / 10.0), str(self.hw_version / 10.0))

    @staticmethod
    def unpack(data: bytes):
        unpacked_data = unpack("<4B2I", data)
        return DeviceInfo(*unpacked_data)


class DeviceCapability:
    def __init__(self, feature, clk, tseg1_min, tseg1_max, tseg2_min, tseg2_max, sjw_max, brp_min, brp_max, brp_inc):
        self.feature = feature
        self.fclk_can = clk
        self.tseg1_min = tseg1_min
        self.tseg1_max = tseg1_max
        self.tseg2_min = tseg2_min
        self.tseg2_max = tseg2_max
        self.sjw_max = sjw_max
        self.brp_min = brp_min
        self.brp_max = brp_max
        self.brp_inc = brp_inc

    def __str__(self):
        return "Feature bitfield: 0x%08x\r\n"\
            "Clock: %u\r\n"\
            "TSEG1: %u - %u\r\n"\
            "TSEG2: %u - %u\r\n"\
            "SJW (max): %u\r\n"\
            "BRP: %u - %u\r\n"\
            %(self.feature, self.fclk_can, self.tseg1_min, self.tseg1_max, self.tseg2_min, self.tseg2_max,\
                self.sjw_max, self.brp_min, self.brp_max)

    @staticmethod
    def unpack(data: bytes):
        unpacked_data = unpack("<10I", data)
        return DeviceCapability(*unpacked_data)