from .Utils import Clients
from fastapi import FastAPI
from fastapi.middleware.gzip import GZipMiddleware


from .Routers import Generate, ImageSearch, Pixiv, Pjsk


app = FastAPI()
app.router.route_class = Clients.RouteErrorHandler
app.add_middleware(GZipMiddleware, minimum_size=10000)
app.include_router(Generate.router)
app.include_router(ImageSearch.router)
app.include_router(Pixiv.router)
app.include_router(Pjsk.router)


@app.on_event("startup")
async def startup_event():
	await Clients.InitClients()



@app.on_event("shutdown")
async def shutdown_event():
	await Clients.CloseClients()



@app.get("/")
async def root():
	return "hello"