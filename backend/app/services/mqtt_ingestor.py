import json
import logging
from typing import Any
import paho.mqtt.client as mqtt
from pydantic import ValidationError

from app.core.config import settings
from app.core.database import getDbConnection
from app.models.reading import ReadingPayload

logger = logging.getLogger("mqtt_ingestor")


class MQTTIngestor:
    def __init__(self) -> None:
        self._client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2, client_id="fastapi_ingestor",)
        self._client.on_connect = self._onConnect
        self._client.on_message = self._onMessage
        self._client.on_disconnect = self._onDisconnect

    def start(self) -> None:
        logger.info( "Connecting to MQTT broker at %s:%s", settings.mqtt_broker_host, settings.mqtt_broker_port,)
        self._client.connect(settings.mqtt_broker_host, settings.mqtt_broker_port, keepalive=60)
        self._client.loop_start()

    def stop(self) -> None:
        logger.info("Stopping MQTT ingestor loop")
        self._client.loop_stop()
        self._client.disconnect()

    def _onConnect(self, client: mqtt.Client, userdata: Any, flags: mqtt.ConnectFlags, reason_code: mqtt.ReasonCode, properties: Any,) -> None:
        if reason_code.is_failure:
            logger.error("Failed to connect to MQTT broker: %s", reason_code)
            return
        logger.info("Connected to MQTT broker; subscribing to '%s' (QoS 1)", settings.mqtt_topic)
        client.subscribe(settings.mqtt_topic, qos=1)

    def _onDisconnect(self, client: mqtt.Client, userdata: Any, disconnect_flags: mqtt.DisconnectFlags, reason_code: mqtt.ReasonCode, properties: Any, ) -> None:
        logger.warning("Disconnected from MQTT broker (reason=%s)", reason_code)

    def _onMessage(self, client: mqtt.Client, userdata: Any, msg: mqtt.MQTTMessage) -> None:
        try:
            rawData = json.loads(msg.payload.decode("utf-8"))
            reading = ReadingPayload.model_validate(rawData)

            # Fallback: extract node name from topic 'farm/<node>/readings' if omitted in JSON
            if not reading.node:
                parts = msg.topic.split("/")
                reading.node = parts[1] if len(parts) >= 3 else "unknown"

            self._persistReading(reading)

        except (UnicodeDecodeError, json.JSONDecodeError) as exc:
            logger.warning("Dropped malformed JSON on topic '%s': %s", msg.topic, exc)
        except ValidationError as exc:
            logger.warning("Dropped invalid reading payload on topic '%s': %s", msg.topic, exc)
        except Exception as exc:
            logger.exception("Unexpected error ingesting MQTT message: %s", exc)

    def _persistReading(self, reading: ReadingPayload) -> None:
        insert_reading_sql = """
            INSERT INTO readings (node, seq, ts, temp, hum, pressure, moisture, anomaly, score)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
            ON CONFLICT (node, seq) DO NOTHING
            RETURNING seq;
        """
        insert_event_sql = """
            INSERT INTO events (node, ts, type, detail)
            VALUES (%s, %s, %s, %s);
        """

        with getDbConnection() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    insert_reading_sql,
                    (
                        reading.node,
                        reading.seq,
                        reading.ts,
                        reading.temp,
                        reading.hum,
                        reading.pressure,
                        reading.moisture,
                        reading.anomaly,
                        reading.score,
                    ),
                )
                inserted = cur.fetchone()

                # Only log an event if the reading was newly inserted (not a duplicate replay)
                if inserted is not None and reading.anomaly:
                    detail = (
                        f"Anomaly detected (score={reading.score:.2f}): "
                        f"T={reading.temp:.1f}C, H={reading.hum:.1f}%, M={reading.moisture:.1f}%"
                    )
                    cur.execute(insert_event_sql, (reading.node, reading.ts, "ANOMALY", detail))

            conn.commit()

        if inserted is not None:
            logger.info("Inserted reading node=%s seq=%d anomaly=%s", reading.node, reading.seq, reading.anomaly)
        else:
            logger.info("Duplicate ignored: node=%s seq=%d", reading.node, reading.seq)


mqtt_ingestor = MQTTIngestor()