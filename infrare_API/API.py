from fastapi import FastAPI, Request
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel
from typing import Optional
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
import time
import requests
import logging
app = FastAPI()


logging.basicConfig(format="%(asctime)s %(message)s", level=logging.INFO)

templates = Jinja2Templates(directory="template")
app.mount('/static', StaticFiles(directory='static'), name='static')


class Model(BaseModel):
    Open: Optional[str] = '-2'
    model: Optional[str] = '-2'
    temp: Optional[str] = '-2'
    air: Optional[str] = '-2'
    speed : Optional[str] = '-2'
    ud: Optional[str] = '-2'
    lr: Optional[str] = '-2'

@app.get("/data/{AHT10_temp}/{AHT10_hum}/{temperature}/{pressure}/{Light}", response_class=HTMLResponse)
async def read_item(request: Request,AHT10_temp:float,AHT10_hum:float,temperature:float,pressure:float,Light:float):
    @app.get("/", response_class=HTMLResponse)
    async def read_item(request: Request):   
        local_time=time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
        return templates.TemplateResponse("infrare.html", {"request": request,"AHT10_temp":AHT10_temp,"AHT10_hum":AHT10_hum,"temperature":temperature,"pressure":pressure,"Light":Light,"local_time":local_time})
@app.post("/Model")
async def get_data(data:Model):
    print("d")
    dic_data=data.dict()
    r = requests.get("http://192.168.3.217/update", params=dic_data)
    print(dic_data)
    return data

if __name__ == "__main__":
    uvicorn.run("API:app", host='192.168.3.254', reload=True, port=9000, debug=True)