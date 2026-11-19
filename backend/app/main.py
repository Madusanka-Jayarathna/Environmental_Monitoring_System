import logging
from contextlib import asynccontextmanager
from typing import AsyncIterator, Optional
from fastapi import FastAPI, Query
from app.services.mqtt_ingestor import mqtt_ingestor
from app.core.database import getDbConnection

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


@app.get("/readings")
async def get_readings(
    node: Optional[str] = Query(None, description="Filter by node name"),
    limit: int = Query(20, ge=1, le=500, description="Max rows to return"),
):
    """Return the latest readings, newest first. Optionally filter by node."""
    with getDbConnection() as conn:
        with conn.cursor() as cur:
            if node:
                cur.execute(
                    "SELECT node, seq, ts, temp, hum, pressure, moisture, anomaly, score "
                    "FROM readings WHERE node = %s ORDER BY ts DESC LIMIT %s",
                    (node, limit),
                )
            else:
                cur.execute(
                    "SELECT node, seq, ts, temp, hum, pressure, moisture, anomaly, score "
                    "FROM readings ORDER BY ts DESC LIMIT %s",
                    (limit,),
                )
            cols = [d[0] for d in cur.description]
            return [dict(zip(cols, row)) for row in cur.fetchall()]
