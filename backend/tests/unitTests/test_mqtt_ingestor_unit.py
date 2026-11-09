import json
from unittest.mock import MagicMock, patch
import paho.mqtt.client as mqtt
from app.services.mqtt_ingestor import MQTTIngestor


def _make_mqtt_message(topic: str, payload_bytes: bytes) -> mqtt.MQTTMessage:
    msg = mqtt.MQTTMessage(topic=topic.encode("utf-8"))
    msg.payload = payload_bytes
    return msg


def test_on_message_extracts_node_from_topic_when_omitted() -> None:
    ingestor = MQTTIngestor()
    raw = json.dumps({"seq": 10, "temp": 24.1, "hum": 55.0, "pressure":1014, "moisture": 38.0}).encode("utf-8")
    msg = _make_mqtt_message("farm/node_alpha/readings", raw)

    with patch.object(ingestor, "_persistReading") as mock_persist:
        ingestor._onMessage(MagicMock(), None, msg)
        mock_persist.assert_called_once()
        saved_reading = mock_persist.call_args[0][0]
        assert saved_reading.node == "node_alpha"
        assert saved_reading.seq == 10


def test_on_message_drops_malformed_json_gracefully() -> None:
    ingestor = MQTTIngestor()
    msg = _make_mqtt_message("farm/node1/readings", b"{corrupt-json")

    with patch.object(ingestor, "_persistReading") as mock_persist:
        ingestor._onMessage(MagicMock(), None, msg)
        mock_persist.assert_not_called()
