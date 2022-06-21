from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel
from typing import Optional

from io import BytesIO
from PIL import Image, ImageFilter
import base64, logging, reprlib, json

from ..Utils.pixivpy_async import AppPixivAPI
from ..Utils import Clients
from .. import Config
import os

logger = logging.getLogger('uvicorn.error')

router = APIRouter(
	prefix="/pixiv",
	route_class=Clients.RouteErrorHandler
)

class ImageResult(BaseModel):
	info: str
	image: Optional[str] = None

async def download_illust(aapi : AppPixivAPI, illust: dict, meta_page = 1) -> str:
	image_url: Optional[str] = None
	if 'original_image_url' in illust['meta_single_page']:
		image_url = illust['meta_single_page'].get('original_image_url')
	elif illust['page_count'] > 0:
		if illust['page_count'] > meta_page - 1:
			image_url = illust['meta_pages'][meta_page - 1]['image_urls'].get('original')
		else:
			meta_page = 1
			image_url = illust['meta_pages'][0]['image_urls'].get('original')
	else:
		image_url = illust['image_urls']['large']
	if not image_url:
		raise HTTPException(status_code=500, detail="未找到图片链接")

	with BytesIO() as img_io:
		await aapi.download(image_url, fname=img_io)
		if illust['x_restrict'] == 0:
			return base64.b64encode(img_io.getvalue())
		else:
			img_io.seek(0)
			img = Image.open(img_io)
			blur_img = img.filter(ImageFilter.GaussianBlur(radius = 0.05 * max(img.size[0], img.size[1]) * illust['x_restrict']))
			w1, h1 = blur_img.size
			top_img = Image.open(os.path.join(Config.BASE_PATH, 'resources/forbidden.png'))
			w2, h2 = top_img.size
			ratio = 0.7 * min(w1 / w2, h1 / h2)
			top_img = top_img.resize((int(w2 * ratio), int(h2 * ratio)), Image.Resampling.LANCZOS)
			w2, h2 = top_img.size
			blur_img.paste(top_img, (int((w1 - w2) / 2), int((h1 - h2) / 2)), top_img)

			with BytesIO() as img_censored:
				if blur_img.mode in ("RGBA", "P"):
					blur_img.save(img_censored, 'png')
				else:
					blur_img.save(img_censored, 'jpeg')
				return base64.b64encode(img_censored.getvalue())



@router.get("/id/", response_model=ImageResult, response_model_exclude_none=True)
async def GetIllustId(id: int = Query(..., gt=0), page: int = Query(1, gt=0)):
	logger.info('Retrieving info <Pixiv>')
	aapi = await Clients.client_pixiv.GetApi()
	json_result = await aapi.illust_detail(id)
	rep = reprlib.Repr()
	rep.maxstring = 100
	logger.info(f'Result <Pixiv>: {rep.repr(json.dumps(json_result))}')
	if 'illust' not in json_result or not json_result['illust']['visible']:
		return {"info": "ID不存在或作者已将其删除"}
	illust = json_result['illust']
	logger.info('Downloading image <Pixiv>')
	image = await download_illust(aapi, illust, page)
	logger.info('Finish downloading image <Pixiv>')
	return {"info": json.dumps(illust), "image": image}
