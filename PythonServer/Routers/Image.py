from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel
from typing import Optional, cast

from io import BytesIO
from PIL import Image, ImageFilter
import base64, logging


from ..Utils.pysaucenao import containers, errors
from ..Utils.pysaucenao.saucenao import SauceNao
from ..Utils import Clients
from .. import Config

logger = logging.getLogger('uvicorn.error')

router = APIRouter(
	prefix="/image",
	route_class=Clients.RouteErrorHandler
)

class ImageResult(BaseModel):
	info: str
	image: Optional[str] = None



@router.get("/saucenao/", response_model=ImageResult, response_model_exclude_none=True)
async def SearchSauce(url: str = Query(..., min_length=1)):
	with BytesIO() as sauce_img:
		logger.info('Begin downloading image <SauceNao>')
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
		try:
			results = await SauceNao(session=Clients.client_aiohttp, 
						api_key=Config.SAUCENAO_TOKEN,
						results_limit=2,
						min_similarity=65.0,
						priority=[5, 6, 41],
						hide=1).from_file(sauce_img)
		except errors.SauceNaoException as e:
			raise HTTPException(status_code=500, detail=e.__class__.__name__ + (': ' + str(e) if str(e) else ''))

	logger.info('Results from SauceNAO <SauceNao>: ' + str(results))
	if len(results.results) > 0:
		response = ImageResult(info='')
		result = results.results[0]
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
		
		if result.type == containers.TYPE_PIXIV:
			result = cast(containers.PixivSource, result)
			response.info += "标题: " + result.title + f" (id: {result.pixiv_id})" + '\n'
			response.info += "作者: " + result.author_name + f" (id: {result.member_id})" + '\n'
			if result.index:
				response.info += "来源: " + result.index + '\n'
			if result.similarity:
				response.info += "相似度: " + str(result.similarity) + '\n'
		
		elif result.type == containers.TYPE_TWITTER:
			result = cast(containers.TwitterSource, result)	
			if (result.title):
				response.info += "标题: " + result.title + '\n'
			response.info += "作者: " + result.author_name + f" (id: {result.twitter_user_id})" + '\n'
			if result.url:
				response.info += "原网址: " + result.url + '\n'
			if result.index:
				response.info += "来源: " + result.index + '\n'
			if result.similarity:
				response.info += "相似度: " + str(result.similarity) + '\n'

		elif result.type == containers.TYPE_BOORU:
			result = cast(containers.BooruSource, result)	
			response.info += "标题: " + result.title + '\n'
			response.info += "作者: " + result.author_name + '\n'
			if result.source:
				response.info += "原网址: " + result.source + '\n'
			if result.url:
				response.info += "Booru链接: " + result.url + '\n'
			if result.index:
				response.info += "来源: " + result.index + '\n'
			if result.similarity:
				response.info += "相似度: " + str(result.similarity) + '\n'
		
		elif result.type == containers.TYPE_VIDEO:
			result = cast(containers.VideoSource, result)
			if result.title:
				response.info += "标题: " + result.title + '\n'
			if result.timestamp:
				response.info += "位置: " + str(result.timestamp)
				if result.episode:
					response.info += " episode: " + str(result.episode)
				response.info += '\n'
			if result.url:
				response.info += "原网址: " + result.url + '\n'
			if result.index:
				response.info += "来源: " + result.index + '\n'
			if result.similarity:
				response.info += "相似度: " + str(result.similarity) + '\n'
		
		else:	
			if result.title:
				response.info += "标题: " + result.title + '\n'
			if result.author_name:
				if isinstance(result.author_name, list):
					response.info += "作者:"
					for name in result.author_name:
						response.info += " " + name
					response.info += '\n'
				else:
					response.info += "作者: " + result.author_name + '\n'
			if result.url:
				response.info += "原网址: " + result.url + '\n'
			if result.index:
				response.info += "来源: " + result.index + '\n'
			if result.similarity:
				response.info += "相似度: " + str(result.similarity) + '\n'

		return response

	else:
		return {"info": "找不到相关图片捏"}