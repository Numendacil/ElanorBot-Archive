import asyncio
import json
from unittest import result
from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel
from typing import Optional

from io import BytesIO
from PIL import Image, ImageFilter
import base64, logging, time



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
	with BytesIO() as search_img:
		logger.info('Begin downloading target image <SauceNao>')
		async with Clients.client_retry.get(url) as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1280, 1280), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(search_img, 'png')
				else:
					img.save(search_img, 'jpeg')
				search_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <SauceNao>')
		results = await PicImageSearch.SauceNAO(
			client=Clients.client_image_proxy,
			api_key=Config.SAUCENAO_TOKEN,
			numres=3,
			minsim=70,
			dbmaski=0x4000000,
			hide=1
		).search(file=search_img)
		

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


		response = ImageResult(info='')
		result = results.raw[0]
		logger.info(f"Best sauce: {json.dumps(result.origin)}")
		if result.similarity < 65:
			return {"info": "找不到相关图片捏"}

		
		async with Clients.client_retry.get(result.thumbnail) as thumbnail:
			if thumbnail.ok:
				thumbnail_img = Image.open(BytesIO(await thumbnail.read()))
				if result.hidden:
					thumbnail_img = thumbnail_img.filter(ImageFilter.GaussianBlur(radius=0.04 * max(thumbnail_img.size[0], thumbnail_img.size[1])))
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
			response.info += f"作者: @{result.origin['data'].get('twitter_user_handle')} (id: {result.origin['data'].get('twitter_user_id')})\n"
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
	with BytesIO() as search_img:
		logger.info('Begin downloading target image <Ascii2D>')
		async with Clients.client_retry.get(url) as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1280, 1280), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(search_img, 'png')
				else:
					img.save(search_img, 'jpeg')
				search_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <Ascii2D>')
		results = await PicImageSearch.Ascii2D(client=Clients.client_image_proxy, bovw=True).search(file=search_img)

	result: model.Ascii2DItem = None
	for item in results.raw:
		if item.url:
			result = item
			break

	if result is None:
		return {"info": "找不到相关图片捏"}

	response = ImageResult(info='')
	async with Clients.client_retry.get(result.thumbnail) as thumbnail:
		if thumbnail.ok:
			thumbnail_img = Image.open(BytesIO(await thumbnail.read()))
			thumbnail_img = thumbnail_img.filter(ImageFilter.GaussianBlur(radius=0.04 * max(thumbnail_img.size[0], thumbnail_img.size[1])))
			with BytesIO() as buffered:
				if thumbnail_img.mode in ("RGBA", "P"):
					thumbnail_img.save(buffered, 'png')
				else:
					thumbnail_img.save(buffered, 'jpeg')
				response.image = base64.b64encode(buffered.getvalue())
	
	if result.title:
		response.info += f"标题: {result.title}\n"
	if result.author:
		response.info += f"作者: {result.author}\n"
	response.info += f"网址: {result.url}\n"
	if result.mark:
		response.info += f"来源: {result.mark}\n"
	return response
	
	

@router.get("/trace-moe/", response_model=ImageResult, response_model_exclude_none=True)
async def SearchTraceMoe(url: str = Query(..., min_length=1)):
	with BytesIO() as search_img:
		logger.info('Begin downloading target image <TraceMoe>')
		async with Clients.client_retry.get(url) as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1280, 1280), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(search_img, 'png')
				else:
					img.save(search_img, 'jpeg')
				search_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <TraceMoe>')
		results = await PicImageSearch.TraceMoe(client=Clients.client_image).search(file=search_img)

	if results.error:
		raise HTTPException(status_code=503, detail=results.error)

	if len(results.raw) == 0:
		return {"info": "找不到相关内容捏"}
		
	result = results.raw[0]
	logger.info(f"Best result: {json.dumps({'origin': result.origin, 'data': result.anime_info})}")
	if result.similarity < 85:
		return {"info": "找不到相关内容捏"}

	response = ImageResult(info='')

	translate: str = next((s for s in [result.title_chinese, result.title_english, result.title_romaji] if s), '')
	response.info += f"标题: {result.title_native} {f'({translate})' if translate else ''}\n"
	response.info += f"类型: {result.type} {result.format}\n"
	response.info += f"放送时间: {result.start_date.get('year', '-')}/{result.start_date.get('month', '-')}/{result.start_date.get('day', '-')} - {result.end_date.get('year', '-')}/{result.end_date.get('month', '-')}/{result.end_date.get('day', '-')}\n"
	minutes, seconds = divmod(round(result.From), 60)
	hours, minutes = divmod(minutes, 60)
	if hours > 0:
		timestamp = f"{hours:0>2d}:{minutes:0>2d}:{seconds:0>2d}"
	else:
		timestamp = f"{minutes:0>2d}:{seconds:0>2d}"
	response.info += f"位置: {timestamp}   episode: {result.episode}\n"
	response.info += f"相似度: {result.similarity}\n"

	async with Clients.client_retry.get(result.image) as thumbnail:
		async with Clients.client_retry.get(result.cover_image) as cover:
			if thumbnail.ok and cover.ok:
				thumbnail_img = Image.open(BytesIO(await thumbnail.read()))
				cover_img = Image.open(BytesIO(await cover.read()))
				thumbnail_img = thumbnail_img.resize((round(cover_img.height * thumbnail_img.width / thumbnail_img.height), cover_img.height), Image.Resampling.LANCZOS)
				result_img = Image.new(thumbnail_img.mode, (cover_img.width + thumbnail_img.width + 5, cover_img.height), 'white')
				result_img.paste(thumbnail_img, (0, 0))
				result_img.paste(cover_img, (thumbnail_img.width + 5, 0))

				if result.isAdult:
					result_img = result_img.filter(ImageFilter.GaussianBlur(radius=0.04 * max(result_img.size[0], result_img.size[1])))
				
				with BytesIO() as buffered:
					if result_img.mode in ("RGBA", "P"):
						result_img.save(buffered, 'png')
					else:
						result_img.save(buffered, 'jpeg')
					response.image = base64.b64encode(buffered.getvalue())

	return response
		


async def Test():
	await Clients.InitClients()

	with BytesIO() as search_img:
		logger.info('Begin downloading target image <TraceMoe>')
		async with Clients.client_retry.get('http://gchat.qpic.cn/gchatpic_new/3332614209/852085810-2733700096-B4C917FC2B3BF57EB8F8CA7D69A66D0C/0?term=2') as response:
			if response.ok:
				img = Image.open(BytesIO(await response.read()))
				img.thumbnail((1280, 1280), Image.Resampling.LANCZOS)

				if img.mode in ("RGBA", "P"):
					img.save(search_img, 'png')
				else:
					img.save(search_img, 'jpeg')
				search_img.seek(0)
			else:
				raise HTTPException(status_code=404, detail="无法获取图片")
		logger.info('Downloading finished <TraceMoe>')
		results = await PicImageSearch.TraceMoe(client=Clients.client_image).search(file=search_img)

	if results.error:
		raise HTTPException(status_code=503, detail=results.error)

	if len(results.raw) == 0:
		return {"info": "找不到相关内容捏"}
		
	result = results.raw[0]
	if result.similarity < 85:
		return {"info": "找不到相关内容捏"}

	response = ImageResult(info='')

	translate: str = next((s for s in [result.title_chinese, result.title_english, result.title_romaji] if s), '')
	response.info += f"标题: {result.title_native} {f'({translate})' if translate else ''}\n"
	response.info += f"类型: {result.type} {result.format}\n"
	response.info += f"放送时间: {result.start_date.get('year', '-')}/{result.start_date.get('month', '-')}/{result.start_date.get('day', '-')} - {result.end_date.get('year', '-')}/{result.end_date.get('month', '-')}/{result.end_date.get('day', '-')}\n"
	minutes, seconds = divmod(round(result.From), 60)
	hours, minutes = divmod(minutes, 60)
	if hours > 0:
		timestamp = f"{hours:0>2d}:{minutes:0>2d}:{seconds:0>2d}"
	else:
		timestamp = f"{minutes:0>2d}:{seconds:0>2d}"
	response.info += f"位置: {timestamp}   episode: {result.episode}\n"
	response.info += f"相似度: {result.similarity}\n"

	async with Clients.client_retry.get(result.image) as thumbnail:
		async with Clients.client_retry.get(result.cover_image) as cover:
			if thumbnail.ok and cover.ok:
				thumbnail_img = Image.open(BytesIO(await thumbnail.read()))
				cover_img = Image.open(BytesIO(await cover.read()))
				thumbnail_img.resize((round(cover_img.height * thumbnail_img.width / thumbnail_img.height), cover_img.height), Image.Resampling.LANCZOS)
				result_img = Image.new(thumbnail_img.mode, (cover_img.width + thumbnail_img.width + 5, cover_img.height), 'white')
				result_img.paste(thumbnail_img, (0, 0))
				result_img.paste(cover_img, (thumbnail_img.width + 5, 0))

				if result.isAdult:
					result_img = result_img.filter(ImageFilter.GaussianBlur(radius=0.04 * max(result_img.size[0], result_img.size[1])))
				
				with BytesIO() as buffered:
					if result_img.mode in ("RGBA", "P"):
						result_img.save(buffered, 'png')
					else:
						result_img.save(buffered, 'jpeg')
					response.image = base64.b64encode(buffered.getvalue())

	print(response.info)

	await Clients.CloseClients()

if __name__ == '__main__':
	asyncio.run(Test())