from PIL import Image
from .saveGif import save_transparent_gif
from pkg_resources import resource_stream

frames = 5
resolution = (112, 112)
delay = 60

def make(source, dest):
	"""

	:param source: A filename (string), pathlib.Path object or a file object. (This parameter corresponds
				   and is passed to the PIL.Image.open() method.)
	:param dest: A filename (string), pathlib.Path object or a file object. (This parameter corresponds
				   and is passed to the PIL.Image.save() method.)
	:return: None
	"""
	images = []
	base = Image.open(source).convert('RGBA').resize(resolution)

	for i in range(frames):
		squeeze = i ** 1.5 if i < frames/2 else (frames - i) ** 1.5
		width = 0.9 + squeeze * 0.05
		height = 0.98 - squeeze * 0.1
		offsetX = (1 - width) * 0.5 + 0.01
		offsetY = (1 - height) + 0.05

		canvas = Image.new('RGBA', size=resolution, color=(0, 0, 0, 0))
		canvas.paste(base.resize((round(width * resolution[0]), round(height * resolution[1]))), (round(offsetX * resolution[0]), round(offsetY * resolution[1])))
		pet = Image.open(resource_stream(__name__, f"img/pet{i}.gif")).convert('RGBA').resize(resolution)
		canvas.paste(pet, mask=pet)
		images.append(canvas)

	save_transparent_gif(images, durations=delay, dest=dest)