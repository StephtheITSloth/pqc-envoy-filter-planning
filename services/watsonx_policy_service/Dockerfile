FROM python:3.11-slim

WORKDIR /app

ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

COPY services/watsonx_policy_service/app ./app

EXPOSE 8080

CMD ["python", "-m", "app.server"]

