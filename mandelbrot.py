#!/usr/bin/env python
from PIL import Image, ImageDraw
from math import log, floor

output = Image.new("RGBA", (1920, 1080), (0, 0, 0, 255))
#output = Image.new("RGBA", (1280, 720), (0, 0, 0, 255))
#output = Image.new("RGBA", (853, 480), (0, 0, 0, 255))
image = ImageDraw.Draw(output)

xScale = output.width / (1 - -2.5)
yScale = output.height / float(1 - -1)
maxIteration = 1000
bailout = (2 ** 8) ** 2

pallet = [(0, i * 2, i * 4 + 20, 255) for i in xrange(0, 59)] + [(i, 236 + int(i * 13.3684210526), max(138 - i, 0), 255) for i in xrange(1, 255)] + \
	[(255 - i, i, i, 255) for i in xrange(1, 255)] + [(i, 255, 255 - i, 255) for i in xrange(1, 255)] + \
	[(255, 255, i, 255) for i in xrange (0, 179)] + [(0, 0, 0, 255), (0, 0, 0, 255)]

def linearInterpolate(a, b, amount):
	def colourMul(col, n):
		return tuple(col[i] * n for i in xrange(0, len(col)))

	def colourAdd(a, b):
		return tuple(int(a[i] + b[i]) for i in xrange(0, len(a)))

	assert len(a) == len(b)
	a = colourMul(a, 1 - amount)
	b = colourMul(b, amount)
	return colourAdd(a, b)

for y in xrange(0, 1080):
	for x in xrange(0, 1920):
		x0, y0 = (x / xScale) - 2.5, (y / yScale) - 1
		xn, yn = 0.0, 0.0
		iteration = 0

		while iteration < maxIteration:
			xx, yy = xn ** 2, yn ** 2
			if xx + yy >= bailout:
				break
			yn = 2 * xn * yn + y0
			xn = xx - yy + x0
			iteration += 1

		if iteration < maxIteration:
			zn = log((xn ** 2) + (yn ** 2)) / 2
			nu = log(zn / log(2)) / log(2)
			iteration += 1 - nu

		colour = linearInterpolate(pallet[int(iteration)], pallet[int(iteration) + 1], iteration % 1)
		image.point([(x, y)], fill = colour)

del image
output.save("mandelbrot.png", "PNG")
