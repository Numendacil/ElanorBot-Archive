import base64, logging

from fastapi import APIRouter, HTTPException, Query
from pydantic import BaseModel

from io import BytesIO
from PIL import Image

from ..Utils.petpetgif import petpet
from ..Utils.ChoyenGen import generator
from ..Utils import Clients

logger = logging.getLogger('uvicorn')

class GeneratorResult(BaseModel):
	result: str

router = APIRouter(
	prefix="/gen",
	route_class=Clients.RouteErrorHandler
)

@router.get("/pet/", response_model=GeneratorResult)
async def GeneratePet(qq: int = Query(..., ge=0)):
	logger.info('Generating gif <Petpet>')
	async with Clients.client_retry.get(f'http://q1.qlogo.cn/g?b=qq&nk={qq}&s=640') as profile:
		if profile.ok:
			with BytesIO() as buffered:
				petpet.make(BytesIO(await profile.read()), buffered)
				img_str = base64.b64encode(buffered.getvalue())
			return {"result": img_str}
		else:
			raise HTTPException(status_code=404, detail="无法获取用户头像")



@router.get("/choyen/", response_model=GeneratorResult)
async def GenerateChoyen(upper: str = Query(..., min_length=1), lower: str = Query(..., min_length=1)):
	logger.info('Generating image <Choyen>')
	img = generator.genImage(word_a=upper, word_b=lower)
	with BytesIO() as buffered:
		img.resize((int(img.size[0] * 0.75), int(img.size[1] * 0.75)), Image.Resampling.LANCZOS).save(buffered, format='jpeg')
		img_str = base64.b64encode(buffered.getvalue())
	return {"result": img_str}