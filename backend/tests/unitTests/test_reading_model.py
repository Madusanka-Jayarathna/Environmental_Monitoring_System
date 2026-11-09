from datetime import timezone
import pytest
from pydantic import ValidationError
from app.models.reading import ReadingPayload


def test_valid_reading_payload_with_defaults() -> None:
    payload = ReadingPayload(
        node="node1",
        seq=42,
        temp=25.5,
        hum=60.0,
        pressure=1014.0,
        moisture=45.2,
    )
    assert payload.node == "node1"
    assert payload.seq == 42
    assert payload.anomaly is False
    assert payload.score == 0.0
    assert payload.ts.tzinfo == timezone.utc


def test_negative_sequence_number_rejected() -> None:
    with pytest.raises(ValidationError):
        ReadingPayload(
            node="node1",
            seq=-1,
            temp=25.5,
            hum=60.0,
            pressure=1014.0,
            moisture=45.2,
        )


def test_missing_sensor_fields_rejected() -> None:
    with pytest.raises(ValidationError):
        ReadingPayload(node="node1", seq=1, temp=25.0)  # type: ignore[call-arg]
