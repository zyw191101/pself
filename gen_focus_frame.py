#!/usr/bin/env python3
# 生成 0x45 焦距命令测试帧（HostUART 协议）
# 用法: python3 gen_focus_frame.py <sensor_id> <focus>
#   sensor_id: 0=TV, 1=IR
#   focus: 0~65535
import sys


def build_frame(sensor_id: int, focus: int) -> bytes:
    if sensor_id not in (0, 1):
        raise ValueError("sensor_id 必须是 0(TV) 或 1(IR)")
    if not 0 <= focus <= 0xFFFF:
        raise ValueError("focus 必须在 0~65535 之间")
    params = bytes([sensor_id, focus & 0xFF, (focus >> 8) & 0xFF])  # 小端
    command_id = 0x45
    length = 1 + 1 + 1 + 1 + len(params) + 1  # head+addr+len+cmd+params+crc
    crc = (command_id + sum(params)) & 0xFF
    return bytes([0xCC, 0x06, length, command_id]) + params + bytes([crc])


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__ or "usage: gen_focus_frame.py <sensor_id> <focus>")
        sys.exit(1)
    frame = build_frame(int(sys.argv[1], 0), int(sys.argv[2], 0))
    print(" ".join(f"{b:02X}" for b in frame))
