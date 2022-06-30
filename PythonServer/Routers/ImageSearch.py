import asyncio
import json
from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel
from typing import Optional, cast

from io import BytesIO
from PIL import Image, ImageFilter
import base64, logging



from ..Utils import PicImageSearch
from ..Utils.PicImageSearch import model
from ..Utils import Clients
from .. import Config

logger = logging.getLogger('uvicorn.error')

router = APIRouter(
	prefix="/image-search",
	route_class=Clients.RouteErrorHandler
)

class ImageResult(BaseModel):
	info: str
	image: Optional[str] = None


SAUCE_INDEXES = {
	0: 'H-Magazines',
	2: 'H-Game CG',
	5: 'Pixiv',
	6: 'Pixiv (Historical)',
	8: 'Nico Nico Seiga',
	9: 'Danbooru',
	10: 'drawr Images',
	11: 'Nijie Images',
	12: 'Yande.re',
	16: 'FAKKU',
	18: 'H-Misc',
	19: '2D-Market',
	20: 'MediBang',
	21: 'Anime',
	22: 'H-Anime',
	23: 'Movies',
	24: 'Shows',
	25: 'Gelbooru',
	26: 'Konachan',
	27: 'Sankaku Channel',
	28: 'Anime-Pictures.net',
	29: 'e621.net',
	30: 'Idol Complex',
	31: 'bcy.net Illust',
	32: 'bcy.net Cosplay',
	33: 'PortalGraphics.net (Hist)',
	34: 'deviantArt',
	35: 'Pawoo.net',
	36: 'Madokami (Manga)',
	37: 'MangaDex',
	38: 'E-Hentai',
	39: 'ArtStation',
	40: 'FurAffinity',
	41: 'Twitter',
	42: 'Furry Network',
	43: 'Kemono',
	44: 'Skeb'
}



@router.get("/saucenao/", response_model=ImageResult, response_model_exclude_none=True)
async def SearchSauce(url: str = Query(..., min_length=1)):
	with BytesIO() as sauce_img:
		logger.info('Begin downloading target image <SauceNao>')
		async with Clients.client_retry.get(url) as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1080, 1080), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(sauce_img, 'png')
				else:
					img.save(sauce_img, 'jpeg')
				sauce_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <SauceNao>')
		results = await PicImageSearch.SauceNAO(
			client=Clients.client_image,
			api_key=Config.SAUCENAO_TOKEN,
			numres=3,
			minsim=70,
			dbmaski=0x4000000,
			hide=1
		).search(file=sauce_img)
		

	logger.info(f'Results returned from SauceNAO <SauceNao>: count {results.results_returned}, short {results.short_remaining}, long {results.long_remaining}')
	if results.results_returned > 0:

		# Sort results
		priority_list = [5, 6, 41]	# Pixiv, Pixiv(history), Twitter
		priority_compensate = 5
		def GetKey(item : model.SauceNAOItem) -> float:
			if item.index_id in priority_list:
				return item.similarity + priority_compensate
			else:
				return item.similarity
		results.raw.sort(key= GetKey, reverse=True)

		if  results.raw[0].similarity < 65:
			return {"info": "找不到相关图片捏"}


		response = ImageResult(info='')
		result = results.raw[0]
		logger.info(f"Best sauce: {json.dumps(result.origin)}")
		async with Clients.client_retry.get(result.thumbnail) as thumbnail:
			if thumbnail.ok:
				thumbnail_img = Image.open(BytesIO(await thumbnail.read()))
				if result.hidden:
					thumbnail_img = thumbnail_img.filter(ImageFilter.GaussianBlur(radius=0.05 * max(img.size[0], img.size[1])))
				with BytesIO() as buffered:
					if thumbnail_img.mode in ("RGBA", "P"):
						thumbnail_img.save(buffered, 'png')
					else:
						thumbnail_img.save(buffered, 'jpeg')
					response.image = base64.b64encode(buffered.getvalue())
		
		if result.index_id in [5, 6]:	# Pixiv
			response.info += f"标题: {result.title} (id: {result.pixiv_id})\n"
			response.info += f"作者: {result.author} (id: {result.member_id})\n"
			response.info += f"网址: {result.url}\n"
		
		elif result.index_id in [41]:	# Twitter
			response.info += f"标题: {result.title}\n"
			response.info += f"作者: {result.author} (id: {result.origin['data'].get('twitter_user_id')})\n"
			response.info += f"网址: {result.url}\n"

		elif result.index_id in [9, 12, 25, 26, 29]:	# Booru
			response.info += f"标题: {result.title}\n"
			response.info += f"作者: {result.author}\n"
			if 'source' in result.origin['data']:
				response.info += f"原网址: {result.origin['data'].get('source')}\n"
			response.info += f"Booru链接: {result.url}\n"
		
		elif result.index_id in [21, 22, 23, 24]:	# Video
			response.info += f"标题: {result.title}\n"
			response.info += f"网址: {result.url}\n"
			if 'est_time' in result.origin['data']:
				response.info += f"位置: {result.origin['data'].get('est_time')}"
				if 'part' in result.origin['data']:
					response.info += f"   part {result.origin['data'].get('part')}"
				response.info += '\n'
			if 'year' in result.origin['data']:
				response.info += f"年份: {result.origin['data'].get('year')}\n"
		
		else:	
			response.info += f"标题: {result.title}\n"
			response.info += f"作者: {result.author}\n"
			if result.url:
				response.info += "原网址: " + result.url + '\n'

		response.info += f"来源: {SAUCE_INDEXES[result.index_id]}\n"
		response.info += f"相似度: {result.similarity}\n"
		return response

	else:
		return {"info": "找不到相关图片捏"}



@router.get("/ascii2d/", response_model=ImageResult, response_model_exclude_none=True)
async def SearchAscii2d(url: str = Query(..., min_length=1)):
	with BytesIO() as sauce_img:
		logger.info('Begin downloading target image <Ascii2D>')
		async with Clients.client_retry.get(url) as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1080, 1080), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(sauce_img, 'png')
				else:
					img.save(sauce_img, 'jpeg')
				sauce_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <Ascii2D>')
		results = await PicImageSearch.Ascii2D(client=Clients.client_image, bovw=True).search(file=sauce_img)

	result: model.Ascii2DItem = None
	for item in results.raw:
		if item.url != "":
			result = item
			break

	if result is None:
		return {"info": "找不到相关图片捏"}

	response = ImageResult(info='')
	async with Clients.client_retry.get(result.thumbnail) as thumbnail:
			if thumbnail.ok:
				thumbnail_img = Image.open(BytesIO(await thumbnail.read()))
				thumbnail_img = thumbnail_img.filter(ImageFilter.GaussianBlur(radius=0.08 * max(img.size[0], img.size[1])))
				with BytesIO() as buffered:
					if thumbnail_img.mode in ("RGBA", "P"):
						thumbnail_img.save(buffered, 'png')
					else:
						thumbnail_img.save(buffered, 'jpeg')
					response.image = base64.b64encode(buffered.getvalue())
	
	if result.title != "":
		response.info += f"标题: {result.title}\n"
	if result.author != "":
		response.info += f"作者: {result.author}\n"
	response.info += f"网址: {result.url}\n"
	if result.mark != "":
		response.info += f"来源: {result.mark}\n"
	return response
	
	

async def Test():
	await Clients.InitClients()

	with BytesIO() as sauce_img:
		logger.info('Begin downloading target image <SauceNao>')
		async with Clients.client_retry.get('https://pb.nichi.co/hello-fresh-snake') as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1080, 1080), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(sauce_img, 'png')
				else:
					img.save(sauce_img, 'jpeg')
				sauce_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <SauceNao>')
		resp = await PicImageSearch.Ascii2D(client=Clients.client_image, bovw=True).search(file=sauce_img)
		for item in resp.raw:
			print(item.origin)

	await Clients.CloseClients()

if __name__ == '__main__':
	asyncio.run(Test())