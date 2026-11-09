import logging
from contextlib import asynccontextmanager
from typing import AsyncIterator
from fastapi import FastAPI
from app.services.mqtt_ingestor import mqtt_ingestor

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
)


@asynccontextmanager
async def lifespan(app: FastAPI) -> AsyncIterator[None]:
    mqtt_ingestor.start()
    yield
    mqtt_ingestor.stop()


app = FastAPI(
    title="Edge-First Environmental Monitoring API",
    version="0.2.0",
    lifespan=lifespan,
)


@app.get("/health")
async def health_check() -> dict[str, str]:
    return {"status": "ok"}
