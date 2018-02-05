#!/usr/bin/env python

from PIL import Image, ImageDraw
from math import fmod, floor

maxIterations = 1000

#Credit for this goes to MHeasell and his excellent fmandel on GitHub
# https://github.com/MHeasell/fmandel/

colours = (
	(0x07, 0x00, 0x5D),
	(0x11, 0x19, 0x87),
	(0x1E, 0x4A, 0xAC),
	(0x43, 0x76, 0xCD),
	(0x86, 0xAF, 0xE1),
	(0xD0, 0xE8, 0xF7),
	(0xED, 0xE7, 0xBE),
	(0xF5, 0xC9, 0x5A),
	(0xFD, 0xA8, 0x01),
	(0xC8, 0x81, 0x01),
	(0x94, 0x54, 0x00),
	(0x64, 0x31, 0x01),
	(0x42, 0x12, 0x06),
	(0x0E, 0x03, 0x0E),
	(0x05, 0x00, 0x26),
	(0x05, 0x00, 0x47)
)

def colourMul(col, n):
	return tuple(col[i] * n for i in xrange(0, len(col)))

def colourAdd(a, b):
	return tuple(int(a[i] + b[i]) for i in xrange(0, len(a)))

def colourSub(a, b):
	return tuple(int(a[i] - b[i]) for i in xrange(0, len(a)))

def linearEase(a, b, amount):
	assert len(a) == len(b)
	b = colourMul(b, amount)
	return colourAdd(a, b)

def colourFor(x):
	normX = fmod(x, len(colours))
	idx = int(normX)
	frac = normX - floor(normX)

	col1 = colours[idx]
	col2 = colours[(idx + 1) % len(colours)]
	diff = colourSub(col2, col1)

	return linearEase(col1, diff, frac)

def shade(x):
	if x >= maxIterations:
		return (0, 0, 0)
	return colourFor(x / 6.0)

output = Image.new("RGBA", (4008, 50), (0, 0, 0, 255))
image = ImageDraw.Draw(output)

for x in xrange(0, 4008):
	image.line([(x, 0), (x, 50)], fill = shade(x / 4.0), width = 1)

del image
output.save("gradient.png", "PNG")
