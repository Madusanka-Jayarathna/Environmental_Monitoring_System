from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    database_url: str = "postgresql://env_user:env_password@postgres:5432/env_monitoring"
    mqtt_broker_host: str = "mosquitto"
    mqtt_broker_port: int = 1883
    mqtt_topic: str = "farm/+/readings"


settings = Settings()