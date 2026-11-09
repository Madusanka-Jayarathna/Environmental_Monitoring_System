import json
import time
import paho.mqtt.publish as publish
import psycopg
from app.core.config import settings


def _wait_for_reading(conn: psycopg.Connection, node: str, seq: int, timeout: float = 3.0) -> tuple | None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT node, seq, temp, hum, moisture, anomaly, score FROM readings WHERE node = %s AND seq = %s;",
                (node, seq),
            )
            row = cur.fetchone()
            if row is not None:
                return row
        time.sleep(0.1)
    return None


def test_b1_schema_tables_and_primary_key_exist(dbConn: psycopg.Connection) -> None:
    with dbConn.cursor() as cur:
        cur.execute(
            """
            SELECT table_name
            FROM information_schema.tables
            WHERE table_schema = 'public' AND table_name IN ('readings', 'events');
            """
        )
        tables = {row[0] for row in cur.fetchall()}
        assert tables == {"readings", "events"}

        cur.execute(
            """
            SELECT a.attname
            FROM pg_index i
            JOIN pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey)
            WHERE i.indrelid = 'readings'::regclass AND i.indisprimary;
            """
        )
        pk_columns = {row[0] for row in cur.fetchall()}
        assert pk_columns == {"node", "seq"}


def test_b3_mqtt_ingestion_and_idempotent_deduplication(dbConn: psycopg.Connection) -> None:
    node = "test_node_dedup"
    seq = 101
    payload = json.dumps(
        {
            "node": node,
            "seq": seq,
            "temp": 27.3,
            "hum": 61.5,
            "pressure": 1014.0,
            "moisture": 44.0,
            "anomaly": False,
            "score": 0.05,
        }
    )

    # 1. First publish -> must insert 1 row
    publish.single(
        topic=f"farm/{node}/readings",
        payload=payload,
        qos=1,
        hostname=settings.mqtt_broker_host,
        port=settings.mqtt_broker_port,
    )
    row = _wait_for_reading(dbConn, node, seq)
    assert row is not None
    assert row[0] == node
    assert row[1] == seq

    # 2. Replay exact same (node, seq) -> must be ignored (count remains 1)
    publish.single(
        topic=f"farm/{node}/readings",
        payload=payload,
        qos=1,
        hostname=settings.mqtt_broker_host,
        port=settings.mqtt_broker_port,
    )
    time.sleep(0.5)

    with dbConn.cursor() as cur:
        cur.execute("SELECT COUNT(*) FROM readings WHERE node = %s AND seq = %s;", (node, seq))
        count = cur.fetchone()[0]
    assert count == 1


def test_b3_anomaly_reading_creates_single_event_even_on_replay(dbConn: psycopg.Connection) -> None:
    node = "test_node_anomaly"
    seq = 202
    payload = json.dumps(
        {
            "node": node,
            "seq": seq,
            "temp": 44.8,
            "hum": 18.0,
            "pressure": 1014.0,
            "moisture": 8.5,
            "anomaly": True,
            "score": 0.94,
        }
    )

    # Publish twice to simulate store-and-forward duplicate replay of an anomaly
    for _ in range(2):
        publish.single(
            topic=f"farm/{node}/readings",
            payload=payload,
            qos=1,
            hostname=settings.mqtt_broker_host,
            port=settings.mqtt_broker_port,
        )
        time.sleep(0.3)

    row = _wait_for_reading(dbConn, node, seq)
    assert row is not None
    assert row[5] is True  # anomaly == True

    with dbConn.cursor() as cur:
        cur.execute("SELECT COUNT(*) FROM events WHERE node = %s AND type = 'ANOMALY';", (node,))
        event_count = cur.fetchone()[0]
    assert event_count == 1