from typing import Iterator
import psycopg
import pytest
from app.core.config import settings


@pytest.fixture
def dbConn() -> Iterator[psycopg.Connection]:
    conn = psycopg.connect(settings.database_url)
    with conn.cursor() as cur:
        cur.execute("DELETE FROM events WHERE node LIKE 'test_%';")
        cur.execute("DELETE FROM readings WHERE node LIKE 'test_%';")
    conn.commit()

    yield conn

    with conn.cursor() as cur:
        cur.execute("DELETE FROM events WHERE node LIKE 'test_%';")
        cur.execute("DELETE FROM readings WHERE node LIKE 'test_%';")
    conn.commit()
    conn.close()