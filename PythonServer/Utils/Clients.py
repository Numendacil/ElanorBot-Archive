from typing import Optional, Callable
from aiohttp import ClientSession
from aiohttp_retry import RetryClient, RandomRetry
from aiohttp_socks import ProxyConnector
from fastapi.exceptions import RequestValidationError
from fastapi.routing import APIRoute
from fastapi import HTTPException, Request, Response
from datetime import datetime, timedelta

from .pixivpy_async import PixivClient, AppPixivAPI
import logging
from .. import Config

logger = logging.getLogger('uvicorn.error')

client_aiohttp: Optional[ClientSession] = None
client_proxy: Optional[ClientSession] = None
client_retry: Optional[RetryClient] = None

class Pixiv:
	client: PixivClient
	api : AppPixivAPI
	last_login : datetime

	def __init__(self, **opts) -> None:
		self.client = PixivClient(**opts)
		self.api = AppPixivAPI(client = self.client.start())
		self.api.set_accept_language('zh-cn')
		self.last_login = datetime.min

	async def GetApi(self) -> AppPixivAPI:
		time_since = datetime.now() - self.last_login
		if time_since > timedelta(seconds=3000):
			self.last_login = datetime.now()
			await self.api.login(refresh_token=Config.PIXIV_TOKEN)
		return self.api


	async def close(self) -> None:
		await self.client.close()

client_pixiv: Optional[Pixiv] = None

async def InitClients() -> None:
	global client_retry
	global client_aiohttp
	global client_proxy
	global client_pixiv

	client_retry = RetryClient(retry_options=RandomRetry(attempts=3, min_timeout=1.0, max_timeout=5.0))
	client_aiohttp = ClientSession()
	client_proxy = ClientSession(connector=ProxyConnector.from_url(Config.PROXY))
	client_pixiv = Pixiv(timeout=120, proxy=Config.PROXY)



async def CloseClients() -> None:
	if client_retry is not None:
		await client_retry.close()
	if client_aiohttp is not None:
		await client_aiohttp.close()
	if client_proxy is not None:
		await client_proxy.close()
	if client_pixiv is not None:
		await client_pixiv.close()



class RouteErrorHandler(APIRoute):
	def get_route_handler(self) -> Callable:
		original_route_handler = super().get_route_handler()

		async def custom_route_handler(request: Request) -> Response:
			try:
				return await original_route_handler(request)
			except Exception as ex:
				logger.warn(ex.__class__.__name__ + (': ' + str(ex) if str(ex) else ''))
				if isinstance(ex, HTTPException) or isinstance(ex, RequestValidationError):
					raise ex

				# wrap error into pretty 500 exception
				raise HTTPException(status_code=500, detail=ex.__class__.__name__ + (': ' + str(ex) if str(ex) else ''))

		return custom_route_handler