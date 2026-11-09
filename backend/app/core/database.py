from contextlib import contextmanager
from typing import Iterator
import psycopg
from app.core.config import settings


@contextmanager
def getDbConnection() -> Iterator[psycopg.Connection]:
    conn = psycopg.connect(settings.database_url)
    try:
        yield conn
    finally:
        conn.close()