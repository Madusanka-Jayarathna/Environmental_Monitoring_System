from datetime import datetime, timezone
from typing import Optional
from pydantic import BaseModel, Field


class ReadingPayload(BaseModel):
    node: Optional[str] = None
    seq: int = Field(..., ge=0)
    ts: datetime = Field(default_factory=lambda: datetime.now(timezone.utc))
    temp: float
    hum: float
    pressure: float
    moisture: float
    anomaly: bool = False
    score: float = 0.0
