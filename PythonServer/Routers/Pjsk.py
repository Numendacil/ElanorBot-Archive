from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from typing import Optional

import json, logging, urllib


from ..Utils import Clients

logger = logging.getLogger('uvicorn.error')

router = APIRouter(
	prefix="/pjsk",
	route_class=Clients.RouteErrorHandler
)

class ImageResult(BaseModel):
	info: str
	image: Optional[str] = None


@router.get("/songinfo/{id}")
async def GetSongInfo(id : int):
	url = f'https://public-api.unijzlsx.com/getalias/{id}'
	async with Clients.client_retry.get(url) as response:
		body = await response.text()
		if "musicid not found" in body:
			raise HTTPException(status_code=404, detail="musicId not found")
	entry = {}
	entry["musicId"] = id
	entry["alias"] = body.encode('raw_unicode_escape').decode('unicode_escape').encode('utf-16', 'surrogatepass').decode('utf-16').split('，')
	url = f'http://public-api.unijzlsx.com/getsongid/{urllib.parse.quote_plus(entry["alias"][0])}'
	async with Clients.client_retry.get(url) as response:
		r = json.loads(await response.text())
	assert(id == r["musicId"])
	entry["title"] = r["title"]
	entry["translate"] = r["translate"]
	return entry