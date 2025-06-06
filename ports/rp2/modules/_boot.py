import vfs
import machine, rp2
import graphics
from graphics import fill, fill_rect, fill_circle, pixel, color, text
from machine import I2S, Pin

# Try to mount the filesystem, and format the flash if it doesn't exist.
# Note: the flash requires the programming size to be aligned to 256 bytes.
bdev = rp2.Flash()
try:
    fs = vfs.VfsLfs2(bdev, progsize=256)
except:
    vfs.VfsLfs2.mkfs(bdev, progsize=256)
    fs = vfs.VfsLfs2(bdev, progsize=256)
vfs.mount(fs, "/")

del vfs, bdev, fs

# Sprig Firmware Start
graphics.init()
fill(0)